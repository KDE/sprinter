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
#include <QPointer>
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

class SessionDataThread : public QThread
{
    Q_OBJECT

public:
    SessionDataThread(QObject *parent);
    void run();
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
    void performLoadRunner(int index);
    void startMatching();
    void endQuerySession();

Q_SIGNALS:
    void requestFurtherMatching();
    void requestSync();
    void loadingRunnerMetaData();
    void loadedRunnerMetaData();
    void busyChanged(int metaDataIndex);
    void requestLoadRunner(int index);
    void runnerLoaded(int index);
    void requestEndQuerySession();
    void resetModel();

public Q_SLOTS:
    void sessionDataRetrieved(const QUuid &sessionId, int, RunnerSessionData *data);
    void startQuery(const QString &query);
    void loadRunner(int index);

private Q_SLOTS:
    void startSync();
    void updateBusyStatus();

private:
    void loadRunnerMetaData();
    void retrieveSessionData(int index);
    bool startNextRunner();
    void clearSessionData();

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
    QUuid m_sessionId;
    QTimer *m_restartMatchingTimer;
    NonRestartingTimer *m_startSyncTimer;
    MatchRunnable *m_dummyMatcher;
    RunnerSessionData *m_dummySessionData;
    int m_matchCount;
    QPointer<SessionDataThread> m_sessionDataThread;
};

class SignalForwarder : public QObject
{
    Q_OBJECT
public:
    SignalForwarder(RunnerManagerThread *thread)
        : QObject(0),
          m_thread(thread)
    {}

public Q_SLOTS:
    void loadRunner(int index) {
        m_thread->performLoadRunner(index);
    }

    void startMatching() {
        m_thread->startMatching();
    }

    void endQuerySession() {
        m_thread->endQuerySession();
    }

private:
    RunnerManagerThread *m_thread;
};

class SessionDataRetriever : public QObject, public QRunnable
{
    Q_OBJECT
public:
    SessionDataRetriever(QThread *destinationThread, const QUuid &sessionId, int index, AbstractRunner *runner);
    void run();

Q_SIGNALS:
    void sessionDataRetrieved(const QUuid &sessionId, int index, RunnerSessionData *data);

private:
    QThread *m_destinationThread;
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

