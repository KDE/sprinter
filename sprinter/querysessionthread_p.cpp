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

#include "querysessionthread_p.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QMetaEnum>
#include <QPluginLoader>
#include <QReadLocker>
#include <QThreadPool>
#include <QTimer>
#include <QTime>
#include <QWriteLocker>

#include <unistd.h>

#include "runner.h"
#include "runner_p.h"
#include "querysession.h"
#include "runnersessiondata_p.h"

namespace Sprinter
{

QString textForEnum(const QObject *obj, const char *enumName, int value)
{
    QMetaEnum e = obj->metaObject()->enumerator(obj->metaObject()->indexOfEnumerator(enumName));
    for (int i = 0; i < e.keyCount(); ++i) {
        if (e.value(i) == value) {
            return e.key(i);
        }
    }

    return "Unknown";
}

QuerySessionThread::QuerySessionThread(QuerySession *parent)
    : QThread(0),
      m_threadPool(new QThreadPool(this)),
      m_session(parent),
      m_dummySessionData(new RunnerSessionData(0)),
      m_dummyMatcher(new MatchRunnable(0,
                                       QSharedPointer<RunnerSessionData>(),
                                       m_context)),
      m_runnerBookmark(-1),
      m_currentRunner(-1),
      m_sessionId(QUuid::createUuid()),
      m_restartMatchingTimer(0),
      m_startSyncTimer(0),
      m_matchCount(-1)
{
    // to synchronize in the thread the session lives in
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
}

QuerySessionThread::~QuerySessionThread()
{
    clearSessionData();

    delete m_dummyMatcher;

    //TODO: this will break if there are threads running
    qDeleteAll(m_runners);

    //TODO: wait for all matching and sessiondata fetch threads to complete?
}

void QuerySessionThread::run()
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

    //FIXME: should not delete right away -> sh
    delete m_sessionDataThread;
    m_sessionDataThread = 0;

    deleteLater();
    qDebug() << "************** WORKER THREAD COMPLETE **************";
}

void QuerySessionThread::syncMatches()
{
    // this looks quite round-about, but allows the timer to
    // be moved to any thread and the right thing happen
    emit requestSync();
}

void QuerySessionThread::startSync()
{
    // TODO: do we need to do the sync in chunks?
    // for large numbers of runners and matches, doing them
    // all at once may be too slow. needs to be measured in future
//     QTime t;
//     t.start();
    m_matchCount = -1;

    int offset = 0;
    foreach (QSharedPointer<RunnerSessionData> data, m_sessionData) {
        if (data) {
            offset += data->d->syncMatches(offset);
        }
    }
    m_matchCount = offset;
//     qDebug() << "synchronization took" << t.elapsed();
}

int QuerySessionThread::matchCount() const
{
    if (m_matchCount < 0) {
        int count = 0;

        foreach (QSharedPointer<RunnerSessionData> data, m_sessionData) {
            if (data) {
                count += data->matches(RunnerSessionData::SynchronizedMatches).size();
            }
        }

        const_cast<QuerySessionThread *>(this)->m_matchCount = count;
    }

    return m_matchCount;
}

QueryMatch QuerySessionThread::matchAt(int index)
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

QVector<RunnerMetaData> QuerySessionThread::runnerMetaData() const
{
     return m_runnerMetaData;
}

void QuerySessionThread::loadRunnerMetaData()
{
    emit loadingRunnerMetaData();

    QTime t;
    t.start();
    m_sessionId = QUuid::createUuid();

    QVector<Runner *> runnersTmp = m_runners;
    m_runners.clear();
    m_enabledRunnerIds.clear();

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

                //TODO: check for having the Runner interface?
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
                m_enabledRunnerIds << md.id;
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

void QuerySessionThread::loadRunner(int index)
{
    //qDebug() << "RUNNING LOADING REQUEST FROM" << QThread::currentThread();
    emit requestLoadRunner(index);
}

void QuerySessionThread::performLoadRunner(int index)
{
    //qDebug() << "RUNNING LOADING OCCURING IN" << QThread::currentThread();
    if (index >= m_runnerMetaData.count() || index < 0) {
        return;
    }

    if (m_runnerMetaData[index].runner) {
        return;
    }

    const QString path = m_runnerMetaData[index].library;
    QPluginLoader loader(path);
    QObject *plugin = loader.instance();
    Runner *runner = qobject_cast<Runner *>(plugin);
    if (runner) {
        m_runnerMetaData[index].busy = false;
        m_runnerMetaData[index].runner = runner;

        m_sessionData[index].clear();
        m_runners[index] = runner;
        runner->d->id = m_runnerMetaData[index].id;
        retrieveSessionData(index);
    } else {
        m_runnerMetaData[index].runner = 0;
        m_runnerMetaData[index].busy = false;
        qWarning() << "LOAD FAILURE" <<  path << ":" << loader.errorString();
        delete plugin;
    }

    emit runnerLoaded(index);
}

void QuerySessionThread::retrieveSessionData(int index)
{
    Runner *runner = m_runners.at(index);

    //qDebug() << runner;
    if (!runner || m_sessionData.at(index)) {
        return;
    }

    if (!m_sessionDataThread) {
        m_sessionDataThread = new SessionDataThread();
    }

    m_sessionDataThread->start();
    m_sessionData[index] = m_dummySessionData;
    SessionDataRetriever *rtrver = new SessionDataRetriever(m_sessionDataThread, m_sessionId, index, runner);
    rtrver->setAutoDelete(true);
    connect(rtrver, SIGNAL(sessionDataRetrieved(QUuid,int,RunnerSessionData*)),
            this, SLOT(sessionDataRetrieved(QUuid,int,RunnerSessionData*)));
    m_threadPool->start(rtrver);
}

void QuerySessionThread::sessionDataRetrieved(const QUuid &sessionId, int index, RunnerSessionData *data)
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
        data->d->associateSession(m_session);
        data->d->enabled = m_enabledRunnerIds.contains(m_runnerMetaData[index].id);
    }

    m_sessionData[index].reset(data);

    if (data) {
        connect(data, SIGNAL(busyChanged(bool)), this, SLOT(updateBusyStatus()));
        {
            QWriteLocker lock(&m_matchIndexLock);
            if (m_runnerBookmark == m_currentRunner) {
                m_runnerBookmark = m_currentRunner == 0 ? m_runners.size() - 1 : m_currentRunner - 1;
            }
        }

        emit requestFurtherMatching();
    }
}

void QuerySessionThread::updateBusyStatus()
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

bool QuerySessionThread::startNextRunner()
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

    // if we have a session data object, we have a runner
    Runner *runner = m_runners.at(m_currentRunner);
    Q_ASSERT(runner);

    //qDebug() << "          created a new matcher";
    matcher = new MatchRunnable(runner, sessionData, m_context);
    if (!m_threadPool->tryStart(matcher)) {
        //qDebug() << "          threads be full";
        delete matcher;
        emit requestFurtherMatching();
        return false;
    }

    m_matchers[m_currentRunner] = matcher;
    return true;
}

void QuerySessionThread::startMatching()
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

void QuerySessionThread::launchDefaultMatches()
{
     m_context.setIsDefaultMatchesRequest(true);
     startQuery();
}

bool QuerySessionThread::launchQuery(const QString &query)
{
    if (m_context.query() == query) {
        return false;
    }

    m_context.setQuery(query);
    startQuery();
    return true;
}

QString QuerySessionThread::query() const
{
    return m_context.query();
}

bool QuerySessionThread::setImageSize(const QSize &size)
{
    if (m_context.imageSize() != size) {
        m_context.setImageSize(size);
        return true;
    }

    return false;
}

QSize QuerySessionThread::imageSize() const
{
    return m_context.imageSize();
}

void QuerySessionThread::startQuery()
{
    //qDebug() << "requesting query from UI thread..." << QThread::currentThread() << m_context.query();

    {
        QWriteLocker lock(&m_matchIndexLock);
        m_runnerBookmark = m_currentRunner == 0 ? m_runners.size() - 1 : m_currentRunner - 1;
        m_matchers.fill(0);
    }

    emit requestFurtherMatching();
}

void QuerySessionThread::clearSessionData()
{
    for (int i = 0; i < m_sessionData.size(); ++i) {
        m_sessionData[i].clear();
    }

    if (m_sessionDataThread) {
        m_sessionDataThread->exit();
    }
}

void QuerySessionThread::endQuerySession()
{
    m_sessionId = QUuid::createUuid();

    QWriteLocker lock(&m_matchIndexLock);

    clearSessionData();
    m_matchers.fill(0);

    m_matchCount = -1;
    m_runnerBookmark = m_currentRunner = 0;
    emit resetModel();
}

void QuerySessionThread::setEnabledRunners(const QStringList &runnerIds)
{
    m_enabledRunnerIds = runnerIds;
    // this loop relies on the (valid) assumption that the session data
    // and metadata vectors are the same size
    const int runnerCount = m_runnerMetaData.count();
    for (int i = 0; i < runnerCount; ++i) {
        QSharedPointer<RunnerSessionData> sessionData = m_sessionData[i];
        if (!sessionData) {
            continue;
        }

        sessionData->setEnabled(m_enabledRunnerIds.contains(m_runnerMetaData[i].id));
    }
}

QStringList QuerySessionThread::enabledRunners() const
{
    return m_enabledRunnerIds;
}

MatchRunnable::MatchRunnable(Runner *runner, QSharedPointer<RunnerSessionData> sessionData, QueryContext &context)
    : m_runner(runner),
      m_sessionData(sessionData),
      m_context(context)
{
}

void MatchRunnable::run()
{
    if (m_sessionData) {
        RunnerSessionData::Busy busy(m_sessionData.data());
        m_sessionData.data()->startMatch(m_context);
    }
}

SessionDataRetriever::SessionDataRetriever(QThread *destinationThread, const QUuid &sessionId, int index, Runner *runner)
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
    Runner *runner = m_match.runner();
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

} // namespace
#include "moc_querysessionthread_p.cpp"