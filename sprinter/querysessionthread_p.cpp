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

#include "runner.h"
#include "runner_p.h"
#include "querysession.h"
#include "runnersessiondata_p.h"

#define DEBUG_THREADING
#define DEBUG_PLUGIN_DISCOVERY

#ifdef DEBUG_THREADING
    #define CHECK_IS_WORKER_THREAD \
        Q_ASSERT_X(QThread::currentThread() != QCoreApplication::instance()->thread(), "QeST thread check", "should be in worker thread, is not");
    #define CHECK_IS_GUI_THREAD \
        Q_ASSERT_X(QThread::currentThread() == QCoreApplication::instance()->thread(), "QeST thread check", "should be in GUI thread, is not");
#else
    #define CHECK_IS_WORKER_THREAD
    #define CHECK_IS_GUI_THREAD
#endif

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

QuerySessionThread::QuerySessionThread(QuerySession *session)
    : QObject(0),
      m_threadPool(new QThreadPool(this)),
      m_session(session),
      m_dummySessionData(new RunnerSessionData(0)),
      m_runnerBookmark(0),
      m_currentRunner(0),
      m_sessionId(QUuid::createUuid()),
      m_restartMatchingTimer(new QTimer(this)),
      m_matchCount(-1)
{
    m_restartMatchingTimer->setInterval(50);
    m_restartMatchingTimer->setSingleShot(true);
    connect(this, SIGNAL(continueMatching()),
            m_restartMatchingTimer, SLOT(start()));
    connect(m_restartMatchingTimer, SIGNAL(timeout()),
            this, SLOT(startMatching()));
}

QuerySessionThread::~QuerySessionThread()
{
    {
        QWriteLocker lock(&m_matchIndexLock);
        clearSessionData();
    }

    m_sessionDataThread->exit();
    m_sessionDataThread = 0;
}

void QuerySessionThread::syncMatches()
{
    CHECK_IS_GUI_THREAD
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
    CHECK_IS_GUI_THREAD

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
    CHECK_IS_GUI_THREAD

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
    CHECK_IS_WORKER_THREAD

    emit loadingRunnerMetaData();

#ifdef DEBUG_PLUGIN_DISCOVERY
    QTime t;
    t.start();
#endif

    m_sessionId = QUuid::createUuid();

    m_runners.clear();
    m_enabledRunnerIds.clear();

    {
        QWriteLocker lock(&m_matchIndexLock);
        clearSessionData();
        m_matchers.clear();
    }

    foreach (const QString &path, QCoreApplication::instance()->libraryPaths()) {
        if (path.endsWith("plugins")) {
            QDir pluginDir(path);
            pluginDir.cd("sprinter");
            foreach (const QString &fileName, pluginDir.entryList(QDir::Files)) {
                const QString path = pluginDir.absoluteFilePath(fileName);
                QPluginLoader loader(path);

                RunnerMetaData md;
                md.library = path;
                md.id = loader.metaData()["IID"].toString();
                if (md.id.isEmpty()) {
                    qDebug() << "Invalid plugin, no metadata:" << path;
                    continue;
                }

                //TODO: these values come from desktoptojson, currently in the
                //      frameworks/kservices repository. Very ugly, discussion
                //      ongoing as to how to make them better.
                QJsonObject json = loader.metaData()["MetaData"].toObject();
                md.name = json["Name"].toString();
                md.description = json["Comment"].toString();
                md.icon = json["Icon"].toString();
                md.license = json["X-KDE-PluginInfo-License"].toString();
                md.author = json["X-KDE-PluginInfo-Author"].toString();
                md.contactEmail = json["X-KDE-PluginInfo-Email"].toString();
                md.version = json["X-KDE-PluginInfo-Version"].toString();
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

#ifdef DEBUG_PLUGIN_DISCOVERY
   qDebug() << m_runnerMetaData.count() << "runner plugins found" << "in" << t.elapsed() << "ms";
#endif
}

void QuerySessionThread::loadRunner(int index)
{
    CHECK_IS_WORKER_THREAD

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
        m_runnerMetaData[index].runner = runner;
        m_runnerMetaData[index].busy = false;
        m_runnerMetaData[index].fetchedSessionData = false;

        m_sessionData[index].clear();
        m_runners[index] = runner;
        runner->d->id = m_runnerMetaData[index].id;
        if (m_context.isValid() &&
            (!m_context.query().isEmpty() ||
             m_context.isDefaultMatchesRequest())) {
            retrieveSessionData(index);
        }
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
    m_runnerMetaData[index].fetchedSessionData = true;

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
        startQuery(false);
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
        if (!m_runnerMetaData[m_currentRunner].fetchedSessionData) {
            retrieveSessionData(m_currentRunner);
        }
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
        emit continueMatching();
        return false;
    }

    m_matchers[m_currentRunner] = matcher;
    return true;
}

void QuerySessionThread::startMatching()
{
    CHECK_IS_WORKER_THREAD
    //qDebug() << m_context.query() << m_currentRunner << m_runnerBookmark;

    if (m_runners.isEmpty()) {
        return;
    }

    if (!m_matchIndexLock.tryLockForWrite()) {
        emit continueMatching();
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
    CHECK_IS_GUI_THREAD

    m_context.setFetchMore(false);
    m_context.setIsDefaultMatchesRequest(true);
    startQuery();
}

bool QuerySessionThread::launchQuery(const QString &query)
{
    CHECK_IS_GUI_THREAD

    const QString oldQuery = m_context.query();
    m_context.setQuery(query);
    if (m_context.query() == oldQuery) {
        return false;
    }

    m_context.setFetchMore(false);
    startQuery();
    return true;
}

void QuerySessionThread::launchMoreMatches()
{
    CHECK_IS_GUI_THREAD

    m_context.setFetchMore(true);
    startQuery(m_currentRunner == m_runnerBookmark);
}

void QuerySessionThread::startQuery(bool clearMatchers)
{
    qDebug() << m_context.query()
             << (m_context.fetchMore() ? "Fetching more." : "")
             << (m_context.isDefaultMatchesRequest() ? "Default matches." : "")
             << (clearMatchers ? "Clearing matchers" : "Keeping Matchers");

    {
        QWriteLocker lock(&m_matchIndexLock);
        m_runnerBookmark = qMax(0, m_currentRunner == 0 ? m_runners.size() - 1
                                                        : m_currentRunner - 1);
        if (clearMatchers) {
            m_matchers.fill(0);
        }
    }

    emit continueMatching();
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

void QuerySessionThread::clearSessionData()
{
    for (int i = 0; i < m_sessionData.size(); ++i) {
        m_sessionData[i].clear();
        m_runnerMetaData[i].fetchedSessionData = false;
    }

    if (m_sessionDataThread) {
        m_sessionDataThread->exit();
    }
}

void QuerySessionThread::endQuerySession()
{
    QWriteLocker lock(&m_matchIndexLock);
    m_sessionId = QUuid::createUuid();

    clearSessionData();
    m_matchers.fill(0);

    m_matchCount = -1;
    m_runnerBookmark = m_currentRunner = 0;
    emit resetModel();
}

void QuerySessionThread::setEnabledRunners(const QStringList &runnerIds)
{
    CHECK_IS_WORKER_THREAD

    if (m_enabledRunnerIds == runnerIds) {
        return;
    }

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

    emit enabledRunnersChanged();
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