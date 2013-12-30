#include "runnermanagerthread_p.h"

#include <QDebug>
#include <QReadLocker>
#include <QThreadPool>
#include <QTimer>
#include <QTime>
#include <QWriteLocker>

#include "abstractrunner.h"
#include "runnermanager.h"

// temporary include for non-pluggable plugins
#include "runners/datetime/datetime.h"
#include "runners/b/b.h"
#include "runners/c/c.h"
#include "runners/d/d.h"

RunnerManagerThread::RunnerManagerThread(RunnerManager *parent)
    : QThread(parent),
      m_manager(parent),
      m_runnerBookmark(-1),
      m_currentRunner(-1),
      m_sessionId(QUuid::createUuid()),
      m_restartMatchingTimer(0),
      m_dummyMatcher(new MatchRunnable(0, 0, m_context))
{
    qRegisterMetaType<QUuid>("QUuid");
}

RunnerManagerThread::~RunnerManagerThread()
{
    const int sessions = m_sessionData.size();
    RunnerSessionData *sessionData;
    for (int i = 0; i < sessions; ++i) {
        sessionData = m_sessionData.at(i);
        if (sessionData) {
            sessionData->deref();
        }
    }

    delete m_dummyMatcher;

    //TODO: this will break if there are threads running
    qDeleteAll(m_runners);

    //TODO: wait for threads to complete?
}

void RunnerManagerThread::run()
{
    connect(m_manager, SIGNAL(queryChanged(QString)),
            this, SLOT(startQuery(QString)));
    qDebug() << "should we do this query, then?" << m_manager->query();

    m_restartMatchingTimer = new QTimer(this);
    m_restartMatchingTimer->setInterval(10);
    m_restartMatchingTimer->setSingleShot(true);
    connect(this, SIGNAL(requestFurtherMatching()),
            m_restartMatchingTimer, SLOT(start()));
    connect(m_restartMatchingTimer, SIGNAL(timeout()),
            this, SLOT(startMatching()));

    loadRunners();
    retrieveSessionData();
    startQuery(m_manager->query());

    exec();

    delete m_restartMatchingTimer;
    m_restartMatchingTimer = 0;
    deleteLater();
    qDebug() << "leaving run";
}

void RunnerManagerThread::loadRunners()
{
    QTime t;
    t.start();
    m_sessionId = QUuid::createUuid();

    //FIXME: will crash if matches are ungoing. reference count?
    qDeleteAll(m_runners);
    m_runners.clear();

    const int sessions = m_sessionData.size();
    RunnerSessionData *sessionData;
    for (int i = 0; i < sessions; ++i) {
        sessionData = m_sessionData.at(i);
        if (sessionData) {
            sessionData->deref();
        }
    }
    m_sessionData.clear();

    m_matchers.clear();

    //TODO: this should be loading from plugins, obviously
    m_runners.append(new DateTimeRunner);
    m_runners.append(new RunnerB);
    m_runners.append(new RunnerC);
    m_runners.append(new RunnerD);

    QWriteLocker lock(&m_matchIndexLock);
    m_currentRunner = m_runnerBookmark = m_runners.isEmpty() ? -1 : 0;
    m_sessionData.resize(m_runners.size());
    m_matchers.resize(m_runners.size());

    qDebug() << m_runners.count() << "runners loaded" << "in" << t.elapsed() << "ms";
}

void RunnerManagerThread::retrieveSessionData()
{
    for (int i = 0; i < m_runners.size(); ++i) {
        AbstractRunner *runner = m_runners.at(i);

        if (m_sessionData.at(i)) {
            continue;
        }

        SessionDataRetriever *rtrver = new SessionDataRetriever(m_sessionId, i, runner);
        rtrver->setAutoDelete(true);
        connect(rtrver, SIGNAL(sessionDataRetrieved(QUuid,int,RunnerSessionData*)),
                this, SLOT(sessionDataRetrieved(QUuid,int,RunnerSessionData*)));
        QThreadPool::globalInstance()->start(rtrver);
    }
}

bool RunnerManagerThread::startNextRunner()
{
    //qDebug() << "    starting for" << m_currentRunner;
    RunnerSessionData *sessionData = m_sessionData.at(m_currentRunner);
    if (!sessionData) {
        m_currentRunner = (m_currentRunner + 1) % m_runners.size();
        //qDebug() << "         no session data";
        return true;
    }

    MatchRunnable *matcher = m_matchers.at(m_currentRunner);
    if (matcher) {
        m_currentRunner = (m_currentRunner + 1) % m_runners.size();
        //qDebug() << "          got a matcher already";
        return true;
    }

    AbstractRunner *runner = m_runners.at(m_currentRunner);

    if (!runner->shouldStartMatch(sessionData, m_context)) {
        //qDebug() << "          skipping";
        matcher = m_dummyMatcher;
    } else {
        matcher = new MatchRunnable(runner, sessionData, m_context);
        matcher->setAutoDelete(true);

        //qDebug() << "          created a new matcher";
        if (!QThreadPool::globalInstance()->tryStart(matcher)) {
            //qDebug() << "          threads be full";
            delete matcher;
            emit requestFurtherMatching();
            return false;
        }
    }

    m_matchers[m_currentRunner] = matcher;
    return true;
}

void RunnerManagerThread::startMatching()
{
    qDebug() << "starting to match with" << m_context.query() << m_currentRunner << m_runnerBookmark;
    if (!m_matchIndexLock.tryLockForWrite()) {
        emit requestFurtherMatching();
        return;
    }

    if (m_currentRunner < 0) {
        m_matchIndexLock.unlock();
        return;
    }

    for (;
         m_currentRunner != m_runnerBookmark;
         m_currentRunner = (m_currentRunner + 1) % m_runners.size()) {
        //m_currentRunner = (m_currentRunner + 1) % m_runners.size();
        if (!startNextRunner()) {
        m_matchIndexLock.unlock();
            return;
        }
    }

    startNextRunner();
    m_matchIndexLock.unlock();
}

void RunnerManagerThread::sessionDataRetrieved(const QUuid &sessionId, int index, RunnerSessionData *data)
{
    qDebug() << "got the data for" << index;

    if (index < 0 || index >= m_sessionData.size()) {
        delete data;
        return;
    }

    RunnerSessionData *oldData = m_sessionData.at(index);
    if (oldData) {
        oldData->deref();
    }

    if (sessionId != m_sessionId) {
        delete data;
        data = 0;
    }

    m_sessionData[index] = data;

    if (data) {
        data->ref();
        emit requestFurtherMatching();
    }
}

void RunnerManagerThread::startQuery(const QString &query)
{
    if (query.isEmpty() || m_query == query) {
        return;
    }

    qDebug() << "kicking off a query ..." << QThread::currentThread() << query;
    m_query = query;
    m_context.setQuery(m_query);

    {
        QWriteLocker lock(&m_matchIndexLock);
        m_runnerBookmark = m_currentRunner == 0 ? m_runners.size() - 1 : m_currentRunner - 1;
        m_matchers.fill(0);
    }

    emit requestFurtherMatching();
}

void RunnerManagerThread::querySessionCompleted()
{
    m_sessionId = QUuid::createUuid();

    QWriteLocker lock(&m_matchIndexLock);

    const int sessions = m_sessionData.size();
    RunnerSessionData *sessionData;
    for (int i = 0; i < sessions; ++i) {
        sessionData = m_sessionData.at(i);
        if (sessionData) {
            sessionData->deref();
        }
    }

    m_sessionData.fill(0);
    m_matchers.fill(0);

    m_runnerBookmark = m_currentRunner = 0;
}

MatchRunnable::MatchRunnable(AbstractRunner *runner, RunnerSessionData *sessionData, RunnerContext &context)
    : m_runner(runner),
      m_sessionData(sessionData),
      m_context(context)
{
    if (m_sessionData) {
        m_sessionData->ref();
    }
}

void MatchRunnable::run()
{
    if (m_sessionData) {
        m_runner->startMatch(m_sessionData, m_context);
        m_sessionData->deref();
    }
}

SessionDataRetriever::SessionDataRetriever(const QUuid &sessionId, int index, AbstractRunner *runner)
    : m_runner(runner),
      m_sessionId(sessionId),
      m_index(index)
{
}

void SessionDataRetriever::run()
{
    RunnerSessionData *session = m_runner->createSessionData();
    emit sessionDataRetrieved(m_sessionId, m_index, session);
}

#include "moc_runnermanagerthread_p.cpp"