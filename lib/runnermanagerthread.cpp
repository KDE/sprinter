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
#include "runners/c/c.h"
#include "runners/youtube/youtube.h"

RunnerManagerThread::RunnerManagerThread(RunnerManager *parent)
    : QThread(parent),
      m_threadPool(new QThreadPool(this)),
      m_manager(parent),
      m_runnerBookmark(-1),
      m_currentRunner(-1),
      m_sessionId(QUuid::createUuid()),
      m_restartMatchingTimer(0),
      m_startSyncTimer(0),
      m_dummyMatcher(new MatchRunnable(0, 0, m_context)),
      m_dummySessionData(new RunnerSessionData(0)),
      m_matchCount(-1)
{
    qRegisterMetaType<QUuid>("QUuid");
    qRegisterMetaType<QUuid>("QueryContext");

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
}

RunnerManagerThread::~RunnerManagerThread()
{
    delete m_restartMatchingTimer;

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

    // can't create this with 'this' as the parent as we are in a different
    // thread at this point.
    m_restartMatchingTimer = new QTimer();
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
    foreach (RunnerSessionData *data, m_sessionData) {
        offset += data->syncMatches(offset);
    }
    m_matchCount = offset;
    qDebug() << "synchronization took" << t.elapsed();
}

int RunnerManagerThread::matchCount() const
{
    if (m_matchCount < 0) {
        int count = 0;

        foreach (RunnerSessionData *data, m_sessionData) {
            count += data->matches(RunnerSessionData::SynchronizedMatches).size();
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
    foreach (RunnerSessionData *data, m_sessionData) {
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
    m_runners.append(new RunnerC);
    m_runners.append(new YoutubeRunner);

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

        m_sessionData[i] = m_dummySessionData;
        SessionDataRetriever *rtrver = new SessionDataRetriever(this, m_sessionId, i, runner);
        rtrver->setAutoDelete(true);
        connect(rtrver, SIGNAL(sessionDataRetrieved(QUuid,int,RunnerSessionData*)),
                this, SLOT(sessionDataRetrieved(QUuid,int,RunnerSessionData*)));
        m_threadPool->start(rtrver);
    }
}

void RunnerManagerThread::sessionDataRetrieved(const QUuid &sessionId, int index, RunnerSessionData *data)
{
    qDebug() << "got session data for runner at index " << index;

    if (index < 0 || index >= m_sessionData.size()) {
        delete data;
        return;
    }

    RunnerSessionData *oldData = m_sessionData.at(index);
    if (oldData && oldData != m_dummySessionData) {
        oldData->deref();
    }

    if (sessionId != m_sessionId) {
        delete data;
        data = 0;
    }

    if (data) {
        data->associateManager(m_manager);
        data->ref();
    }

    m_sessionData[index] = data;

    if (data) {
        emit requestFurtherMatching();
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
        if (!startNextRunner()) {
            m_matchIndexLock.unlock();
            return;
        }
    }

    startNextRunner();
    m_matchIndexLock.unlock();
}

void RunnerManagerThread::startQuery(const QString &query)
{
    if (m_query == query) {
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

MatchRunnable::MatchRunnable(AbstractRunner *runner, RunnerSessionData *sessionData, QueryContext &context)
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

SessionDataRetriever::SessionDataRetriever(RunnerManagerThread *rmt, const QUuid &sessionId, int index, AbstractRunner *runner)
    : m_rmt(rmt),
      m_runner(runner),
      m_sessionId(sessionId),
      m_index(index)
{
}

void SessionDataRetriever::run()
{
    RunnerSessionData *session = m_runner->createSessionData();
    //FIXME: session could be deleted at this point, and then this crashes?
    session->moveToThread(m_rmt);
    emit sessionDataRetrieved(m_sessionId, m_index, session);
}

#include "moc_runnermanagerthread_p.cpp"