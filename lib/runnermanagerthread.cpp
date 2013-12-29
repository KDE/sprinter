#include "runnermanagerthread_p.h"

#include <QDebug>
#include <QThreadPool>

#include "abstractrunner.h"
#include "runnermanager.h"

// temporary include for non-pluggable plugins
#include "runners/a/a.h"
#include "runners/b/b.h"
#include "runners/c/c.h"
#include "runners/d/d.h"

RunnerManagerThread::RunnerManagerThread(RunnerManager *parent)
    : QThread(parent),
      m_manager(parent),
      m_sessionId(QUuid::createUuid())
{
    qRegisterMetaType<QUuid>("QUuid");
}

RunnerManagerThread::~RunnerManagerThread()
{
    QHashIterator<AbstractRunner *, RunnerSessionData *> sessions(m_runnerSessions);
    while (sessions.hasNext()) {
        sessions.next();
        delete sessions.value();
    }

    qDeleteAll(m_runners);
}

void RunnerManagerThread::run()
{
    connect(m_manager, SIGNAL(queryChanged(QString)),
            this, SLOT(startQuery(QString)));
    qDebug() << "should we do this query, then?" << m_manager->query();
    loadRunners();
    retrieveSessionData();
    startQuery(m_manager->query());
    exec();
    deleteLater();
    qDebug() << "leaving run";
}

void RunnerManagerThread::loadRunners()
{
    if (!m_runners.isEmpty()) {
        return;
    }

    //TODO: this should be loading from plugins, obviously
    m_runners.insert(new RunnerA);
    m_runners.insert(new RunnerB);
    m_runners.insert(new RunnerC);
    m_runners.insert(new RunnerD);
    qDebug() << "Runners are loaded" << m_runners.count();
}

void RunnerManagerThread::retrieveSessionData()
{
    QSetIterator<AbstractRunner *> it(m_runners);
    while (it.hasNext()) {
        AbstractRunner *runner = it.next();

        if (m_runnerSessions.contains(runner)) {
            continue;
        }

        SessionDataRetriever *rtrver = new SessionDataRetriever(m_sessionId, runner);
        rtrver->setAutoDelete(true);
        connect(rtrver, SIGNAL(sessionDataRetrieved(QUuid,AbstractRunner*,RunnerSessionData*)),
                this, SLOT(sessionDataRetrieved(QUuid,AbstractRunner*,RunnerSessionData*)));
        QThreadPool::globalInstance()->start(rtrver);
    }
}

void RunnerManagerThread::launchJobs()
{
    QHashIterator<AbstractRunner *, RunnerSessionData *> sessions(m_runnerSessions);
    while (sessions.hasNext()) {
        sessions.next();
        AbstractRunner *runner = sessions.key();
        RunnableMatch *match = m_matchers.value(runner);
        if (!match) {
            // no matcher, so we fetch the match for it
            //TODO: also thread?
            match = runner->createMatcher(sessions.value(), m_context);
        }

        if (match) {
            QThreadPool::globalInstance()->start(match);
        }
    }
}

void RunnerManagerThread::sessionDataRetrieved(const QUuid &sessionId, AbstractRunner *runner, RunnerSessionData *data)
{
    qDebug() << "got the data for" << runner;
    if (!runner || sessionId != m_sessionId) {
        delete data;
        return;
    }

    delete m_runnerSessions.value(runner);
    m_runnerSessions.insert(runner, data);
    //m_activeRunners.insert(runner);
    launchJobs();
}

void RunnerManagerThread::startQuery(const QString &query)
{
    if (query.isEmpty() || m_query == query) {
        return;
    }

    qDebug() << "kicking off a query ..." << QThread::currentThread() << query;
    m_query = query;
    m_context.setQuery(m_query);
    launchJobs();
}

void RunnerManagerThread::querySessionCompleted()
{
    QMutableHashIterator<AbstractRunner *, RunnerSessionData *> it(m_runnerSessions);
    while (it.hasNext()) {
        it.next();
        delete it.value();
        it.value() = 0;
    }
    m_runnerSessions.clear();
}


SessionDataRetriever::SessionDataRetriever(const QUuid &sessionId, AbstractRunner *runner)
    : m_runner(runner),
      m_sessionId(sessionId)
{
}

void SessionDataRetriever::run()
{
    if (m_runner) {
        //FIXME: race condition between check value and using it below
        RunnerSessionData *session = m_runner.data()->createSessionData();
        emit sessionDataRetrieved(m_sessionId, m_runner.data(), session);
    }
}

#include "runnermanagerthread_p.moc"