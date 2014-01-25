/*
 * Copyright (C) 2014 Aaron Seigo <aseigo@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "runnermanagerthread_p.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QPluginLoader>
#include <QReadLocker>
#include <QThreadPool>
#include <QTimer>
#include <QTime>
#include <QWriteLocker>

#include <unistd.h>

#include "abstractrunner.h"
#include "runnermanager.h"

// temporary include for non-pluggable plugins
// #include "runners/datetime/datetime.h"
// #include "runners/c/c.h"
// #include "runners/youtube/youtube.h"

RunnerManagerThread::RunnerManagerThread(RunnerManager *parent)
    : QThread(0),
      m_threadPool(new QThreadPool(this)),
      m_manager(parent),
      m_dummySessionData(new RunnerSessionData(0)),
      m_runnerBookmark(-1),
      m_currentRunner(-1),
      m_sessionId(QUuid::createUuid()),
      m_restartMatchingTimer(0),
      m_startSyncTimer(0),
      m_dummyMatcher(new MatchRunnable(0, QSharedPointer<RunnerSessionData>(), m_context)),
      m_matchCount(-1)
{
    // to synchronize in the thread the manager lives in
    // the timer is created in this parent thread, rather than in
    // run() below
    // if synchronization becomes too slow, it could be moved to happen
    // in this thread with significant complexity
    m_startSyncTimer = new NonRestartingTimer(this);
    m_startSyncTimer->setInterval(10);
    m_startSyncTimer->setSingleShot(true);
    // these connects may look quite round-about, but allows the timer to
    // be moved to any thread and the right thing happen
    connect(this, SIGNAL(requestSync()),
            m_startSyncTimer, SLOT(startIfStopped()));
    connect(m_startSyncTimer, SIGNAL(timeout()),
            this, SLOT(startSync()));
    connect(m_manager, SIGNAL(queryChanged(QString)),
            this, SLOT(startQuery(QString)));
}

RunnerManagerThread::~RunnerManagerThread()
{
    clearSessionData();

    delete m_dummyMatcher;

    //TODO: this will break if there are threads running
    qDeleteAll(m_runners);

    //TODO: wait for all matching and sessiondata fetch threads to complete?
}

void RunnerManagerThread::run()
{
    qDebug() << "************** WORKER THREAD STARTING **************";
    SignalForwarder *forwarder = new SignalForwarder(this);
    // can't create this with 'this' as the parent as we are in a different
    // thread at this point.
    if (!m_restartMatchingTimer) {
        // this timer gets started whenever requestFurtherMatching() is
        // emitted; this may happen from various threads so needs to be
        // triggered by a signal to take advantage of Qt's automagic
        // queueing of signals between threads (you can't call a timer's
        // start() directly from another thread)
        //
        // it gives a small delay to "compress" requests that come in
        // from the UI thread; this way if this worker thread is busy doing
        // something and multiple query updates come in, only the most
        // recent one will be actually processed
        m_restartMatchingTimer = new QTimer();
        m_restartMatchingTimer->setInterval(50);
        m_restartMatchingTimer->setSingleShot(true);
        connect(this, SIGNAL(requestFurtherMatching()),
                m_restartMatchingTimer, SLOT(start()));

        // need to go through the forwarder, which lives in them
        // worker thread, to call startMatching() from the worker thread
        connect(m_restartMatchingTimer, SIGNAL(timeout()),
                forwarder, SLOT(startMatching()));
    }

    // need to go through the forwarder, which lives in them
    // worker thread, to call loadRunner() from the worker thread
    connect(this, SIGNAL(requestLoadRunner(int)),
            forwarder, SLOT(loadRunner(int)));
    connect(this, SIGNAL(requestEndQuerySession()),
            forwarder, SLOT(endQuerySession()));

    loadRunnerMetaData();

    exec();

    delete m_restartMatchingTimer;
    m_restartMatchingTimer = 0;
    delete forwarder;
    deleteLater();
    qDebug() << "************** WORKER THREAD COMPLETE **************";
}

void RunnerManagerThread::syncMatches()
{
    // this looks quite round-about, but allows the timer to
    // be moved to any thread and the right thing happen
    emit requestSync();
}

void RunnerManagerThread::startSync()
{
    // TODO: do we need to do the sync in chunks?
    // for large numbers of runners and matches, doing them
    // all at once may be too slow. needs to be measured in future
    QTime t;
    t.start();
    m_matchCount = -1;

    int offset = 0;
    foreach (QSharedPointer<RunnerSessionData> data, m_sessionData) {
        if (data) {
            offset += data->syncMatches(offset);
        }
    }
    m_matchCount = offset;
    qDebug() << "synchronization took" << t.elapsed();
}

int RunnerManagerThread::matchCount() const
{
    if (m_matchCount < 0) {
        int count = 0;

        foreach (QSharedPointer<RunnerSessionData> data, m_sessionData) {
            if (data) {
                count += data->matches(RunnerSessionData::SynchronizedMatches).size();
            }
        }

        const_cast<RunnerManagerThread *>(this)->m_matchCount = count;
    }

    return m_matchCount;
}

QueryMatch RunnerManagerThread::matchAt(int index)
{
    if (index < 0 || index >= matchCount()) {
        return QueryMatch();
    }

    //qDebug() << "** matchAt" << index;
    QVector<QueryMatch> matches;
    foreach (QSharedPointer<RunnerSessionData> data, m_sessionData) {
        if (!data) {
            continue;
        }

        matches = data->matches(RunnerSessionData::SynchronizedMatches);
        //qDebug() << "    " << matches.size() << index;
        if (matches.size() > index) {
            //qDebug() << "     found" << matches[index].title();
            return matches[index];
        } else {
            index -= matches.size();
        }
    }

    return QueryMatch();
}

QVector<RunnerMetaData> RunnerManagerThread::runnerMetaData() const
{
     return m_runnerMetaData;
}

void RunnerManagerThread::loadRunnerMetaData()
{
    emit loadingRunnerMetaData();

    QTime t;
    t.start();
    m_sessionId = QUuid::createUuid();

    QVector<AbstractRunner *> runnersTmp = m_runners;
    m_runners.clear();
    //FIXME will crash if matches are ongoing
    qDeleteAll(runnersTmp);

    {
        //TODO audit locking around clearSessionData
        QWriteLocker lock(&m_matchIndexLock);
        clearSessionData();
        m_matchers.clear();
    }

    //TODO a little ugly, including the hardcoded "sprinter"
    foreach (const QString &path, QCoreApplication::instance()->libraryPaths()) {
        if (path.endsWith("plugins")) {
            QDir pluginDir(path);
            pluginDir.cd("sprinter");
            foreach (const QString &fileName, pluginDir.entryList(QDir::Files)) {
                const QString path = pluginDir.absoluteFilePath(fileName);
                QPluginLoader loader(path);

                //TODO: check for having the AbstractRunner interface?
                RunnerMetaData md;
                md.library = path;
                md.id = loader.metaData()["IID"].toString();
                if (md.id.isEmpty()) {
                    qDebug() << "Invalid plugin, no metadata:" << path;
                    continue;
                }

                QJsonObject json = loader.metaData()["MetaData"].toObject();
                //TODO localization
                md.name = json["name"].toString();
                md.description = json["description"].toString();
                m_runnerMetaData << md;
            }
        }
    }

    emit loadedRunnerMetaData();

    QWriteLocker lock(&m_matchIndexLock);
    m_currentRunner = m_runnerBookmark = 0;

    m_runners.resize(m_runnerMetaData.size());
    m_sessionData.resize(m_runnerMetaData.size());
    m_matchers.resize(m_runnerMetaData.size());

    qDebug() << m_runnerMetaData.count() << "runner plugins found" << "in" << t.elapsed() << "ms";

}

void RunnerManagerThread::loadRunner(int index)
{
    //qDebug() << "RUNNING LOADING REQUEST FROM" << QThread::currentThread();
    emit requestLoadRunner(index);
}

void RunnerManagerThread::performLoadRunner(int index)
{
    //qDebug() << "RUNNING LOADING OCCURING IN" << QThread::currentThread();
    if (index >= m_runnerMetaData.count() || index < 0) {
        return;
    }

    if (m_runnerMetaData[index].loaded) {
        return;
    }

    const QString path = m_runnerMetaData[index].library;
    QPluginLoader loader(path);
    QObject *plugin = loader.instance();
    AbstractRunnerFactory *runnerFactory = qobject_cast<AbstractRunnerFactory *>(plugin);
    AbstractRunner *runner = runnerFactory ? runnerFactory->create(m_runnerMetaData[index].id, this) : 0;
    if (runner) {
        m_runnerMetaData[index].loaded = true;
        m_sessionData[index].clear();
        m_runners[index] = runner;
        retrieveSessionData(index);
        emit runnerLoaded(index);
    } else {
        m_runnerMetaData[index].loaded = false;
        qWarning() << "LOAD FAILURE" <<  path << ":" << loader.errorString();
        delete plugin;
    }
}

void RunnerManagerThread::retrieveSessionData(int index)
{
    AbstractRunner *runner = m_runners.at(index);

    //qDebug() << runner;
    if (!runner || m_sessionData.at(index)) {
        return;
    }

    if (!m_sessionDataThread) {
        m_sessionDataThread = new SessionDataThread(this);
    }

    m_sessionDataThread->start();
    m_sessionData[index] = m_dummySessionData;
    SessionDataRetriever *rtrver = new SessionDataRetriever(m_sessionDataThread, m_sessionId, index, runner);
    rtrver->setAutoDelete(true);
    connect(rtrver, SIGNAL(sessionDataRetrieved(QUuid,int,RunnerSessionData*)),
            this, SLOT(sessionDataRetrieved(QUuid,int,RunnerSessionData*)));
    m_threadPool->start(rtrver);
}

void RunnerManagerThread::sessionDataRetrieved(const QUuid &sessionId, int index, RunnerSessionData *data)
{
    qDebug() << "got session data for runner at index " << index;

    if (index < 0 || index >= m_sessionData.size()) {
        delete data;
        return;
    }

    if (sessionId != m_sessionId) {
        delete data;
        data = 0;
    }

    if (data) {
        data->associateManager(m_manager);
    }

    m_sessionData[index].reset(data);

    if (data) {
        connect(data, SIGNAL(busyChanged()), this, SLOT(updateBusyStatus()));
        {
            QWriteLocker lock(&m_matchIndexLock);
            if (m_runnerBookmark == m_currentRunner) {
                m_runnerBookmark = m_currentRunner == 0 ? m_runners.size() - 1 : m_currentRunner - 1;
            }
        }

        emit requestFurtherMatching();
    }
}

void RunnerManagerThread::updateBusyStatus()
{
    RunnerSessionData *sessionData = qobject_cast<RunnerSessionData *>(sender());
    if (!sessionData) {
        return;
    }

    for (int i = 0; i < m_sessionData.count(); ++i) {
        if (m_sessionData[i] == sessionData) {
            m_runnerMetaData[i].busy = sessionData->isBusy();
            emit busyChanged(i);
            return;
        }
    }
}

bool RunnerManagerThread::startNextRunner()
{
    //qDebug() << "    starting for" << m_currentRunner;
    QSharedPointer<RunnerSessionData> sessionData = m_sessionData.at(m_currentRunner);
    if (!sessionData) {
        //qDebug() << "         no session data" << m_currentRunner;
        return true;
    }

    MatchRunnable *matcher = m_matchers.at(m_currentRunner);
    if (matcher) {
        //qDebug() << "          got a matcher already";
        return true;
    }

    AbstractRunner *runner = m_runners.at(m_currentRunner);

    if (!sessionData->shouldStartMatch(m_context)) {
        //qDebug() << "          skipping";
        sessionData->setMatches(QVector<QueryMatch>(), m_context);
        matcher = m_dummyMatcher;
    } else {
        matcher = new MatchRunnable(runner, sessionData, m_context);

        //qDebug() << "          created a new matcher";
        if (!m_threadPool->tryStart(matcher)) {
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
    //qDebug() << "starting query in work thread..." << QThread::currentThread() << m_context.query() << m_currentRunner << m_runnerBookmark;

    if (!m_matchIndexLock.tryLockForWrite()) {
        emit requestFurtherMatching();
        return;
    }

    if (m_runners.size() == 1) {
        // with just one runner we don't need to loop at all
        m_currentRunner = 0;
        startNextRunner();
        m_matchIndexLock.unlock();
        return;
    }

    // with multiple runners, we use the currentRunner/m_runnerBookmark
    // state variables to treat the vector as a circular array
    if (m_currentRunner < 0) {
        m_matchIndexLock.unlock();
        return;
    }

    for (;
        m_currentRunner != m_runnerBookmark;
        m_currentRunner = (m_currentRunner + 1) % m_runners.size()) {
        //qDebug() << "Trying with" << m_currentRunner << m_runners[m_currentRunner];
        if (!startNextRunner()) {
            //qDebug() << "STALLED!";
            m_matchIndexLock.unlock();
            return;
        }
    }

    startNextRunner();
    m_matchIndexLock.unlock();
}

void RunnerManagerThread::startQuery(const QString &query)
{
    if (m_context.query() == query) {
        return;
    }

    m_context.setQuery(query);
    //qDebug() << "requesting query from UI thread..." << QThread::currentThread() << query;

    {
        QWriteLocker lock(&m_matchIndexLock);
        m_runnerBookmark = m_currentRunner == 0 ? m_runners.size() - 1 : m_currentRunner - 1;
        m_matchers.fill(0);
    }

    if (!query.isEmpty()) {
        emit requestFurtherMatching();
    }
}

void RunnerManagerThread::clearSessionData()
{
    for (int i = 0; i < m_sessionData.size(); ++i) {
        m_sessionData[i].clear();
    }

    if (m_sessionDataThread) {
        m_sessionDataThread->exit();
    }
}

void RunnerManagerThread::endQuerySession()
{
    m_sessionId = QUuid::createUuid();

    QWriteLocker lock(&m_matchIndexLock);

    clearSessionData();
    m_matchers.fill(0);

    m_matchCount = -1;
    m_runnerBookmark = m_currentRunner = 0;
    emit resetModel();
}

MatchRunnable::MatchRunnable(AbstractRunner *runner, QSharedPointer<RunnerSessionData> sessionData, QueryContext &context)
    : m_runner(runner),
      m_sessionData(sessionData),
      m_context(context)
{
}

void MatchRunnable::run()
{
    if (m_sessionData) {
        RunnerSessionData::Busy busy(m_sessionData.data());
        m_runner->startMatch(m_sessionData.data(), m_context);
    }
}

SessionDataRetriever::SessionDataRetriever(QThread *destinationThread, const QUuid &sessionId, int index, AbstractRunner *runner)
    : m_destinationThread(destinationThread),
      m_runner(runner),
      m_sessionId(sessionId),
      m_index(index)
{
}

void SessionDataRetriever::run()
{
    RunnerSessionData *session = m_runner->createSessionData();
    //FIXME: session could be deleted at this point, and then this crashes?
    session->moveToThread(m_destinationThread);
    emit sessionDataRetrieved(m_sessionId, m_index, session);
}

ExecRunnable::ExecRunnable(const QueryMatch &match, QObject *parent)
    : QObject(parent),
      m_match(match)
{
}

void ExecRunnable::run()
{
    bool success = false;
    //TODO: another race condition with the runner being deleted between this call
    //      and usage
    AbstractRunner *runner = m_match.runner();
    if (runner) {
        success = runner->startExec(m_match);
    }

    emit finished(m_match, success);
}

SessionDataThread::SessionDataThread(QObject *parent)
    : QThread(parent)
{
}

void SessionDataThread::run()
{
    exec();
}

#include "moc_runnermanagerthread_p.cpp"