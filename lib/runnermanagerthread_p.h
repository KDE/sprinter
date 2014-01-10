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

#ifndef RUNNERMANAGERTHREAD
#define RUNNERMANAGERTHREAD

#include <QReadWriteLock>
#include <QRunnable>
#include <QThread>
#include <QTimer>
#include <QVector>
#include <QUuid>

#include "runnermetadata_p.h"
#include "querycontext.h"

class AbstractRunner;
class QThreadPool;
class RunnableMatch;
class RunnerManager;
class RunnerSessionData;

class NonRestartingTimer : public QTimer
{
    Q_OBJECT
//
public:
    NonRestartingTimer(QObject *parent)
        : QTimer(parent)
    {
    }

public Q_SLOTS:
    void startIfStopped()
    {
        if (!isActive()) {
            start();
        }
    }
};

class MatchRunnable : public QRunnable
{
public:
    MatchRunnable(AbstractRunner *runner, RunnerSessionData *sessionData, QueryContext &context);
    void run();

private:
    AbstractRunner *m_runner;
    RunnerSessionData *m_sessionData;
    QueryContext &m_context;
};

class RunnerManagerThread : public QThread
{
    Q_OBJECT

public:
    RunnerManagerThread(RunnerManager *parent = 0);
    ~RunnerManagerThread();

    void run();
    void syncMatches();
    int matchCount() const;
    QueryMatch matchAt(int index);
    QVector<RunnerMetaData> runnerMetaData() const;

Q_SIGNALS:
    void requestFurtherMatching();
    void requestSync();
    void loadingRunnerMetaData();
    void loadedRunnerMetaData();

public Q_SLOTS:
    void sessionDataRetrieved(const QUuid &sessionId, int, RunnerSessionData *data);
    void startQuery(const QString &query);
    void querySessionCompleted();
    void startMatching();

private Q_SLOTS:
    void startSync();

private:
    void loadRunners();
    void retrieveSessionData();
    bool startNextRunner();

    QThreadPool *m_threadPool;
    RunnerManager *m_manager;
    QVector<RunnerMetaData> m_runnerMetaData;
    QVector<AbstractRunner *> m_runners;
    QVector<RunnerSessionData *> m_sessionData;
    QVector<MatchRunnable *> m_matchers;
    QReadWriteLock m_matchIndexLock;
    int m_runnerBookmark;
    int m_currentRunner;
    QueryContext m_context;
    QString m_query;
    QUuid m_sessionId;
    QTimer *m_restartMatchingTimer;
    NonRestartingTimer *m_startSyncTimer;
    MatchRunnable *m_dummyMatcher;
    RunnerSessionData *m_dummySessionData;
    int m_matchCount;
};

class SessionDataRetriever : public QObject, public QRunnable
{
    Q_OBJECT
public:
    SessionDataRetriever(RunnerManagerThread *rmt, const QUuid &sessionId, int index, AbstractRunner *runner);
    void run();

Q_SIGNALS:
    void sessionDataRetrieved(const QUuid &sessionId, int index, RunnerSessionData *data);

private:
    RunnerManagerThread *m_rmt;
    AbstractRunner *m_runner;
    QUuid m_sessionId;
    int m_index;
};

class ExecRunnable : public QObject, public QRunnable
{
    Q_OBJECT
public:
    ExecRunnable(const QueryMatch &match, QObject *parent = 0);
    void run();

Q_SIGNALS:
    void finished(const QueryMatch &match, bool success);

private:
    QueryMatch m_match;
};

#endif

