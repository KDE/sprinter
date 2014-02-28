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

#ifndef QUERYSESSIONTHREAD
#define QUERYSESSIONTHREAD

#include <QReadWriteLock>
#include <QRunnable>
#include <QPointer>
#include <QThread>
#include <QTimer>
#include <QVector>
#include <QUuid>

#include "runnermetadata_p.h"
#include "querycontext.h"

class QThreadPool;

namespace Sprinter
{

class Runner;
class RunnableMatch;
class QuerySession;
class RunnerSessionData;

QString textForEnum(const QObject *obj, const char *enumName, int value);

class NonRestartingTimer : public QTimer
{
    Q_OBJECT

public:
    NonRestartingTimer(QObject *parent = 0)
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
    MatchRunnable(Runner *runner, QSharedPointer<RunnerSessionData> sessionData, QueryContext &context);
    void run();

private:
    Runner *m_runner;
    QSharedPointer<RunnerSessionData> m_sessionData;
    QueryContext &m_context;
};

class SessionDataThread : public QThread
{
    Q_OBJECT

public:
    SessionDataThread(QObject *parent = 0);
    void run();
};

class QuerySessionThread : public QObject
{
    Q_OBJECT

public:
    QuerySessionThread(QuerySession *session);
    ~QuerySessionThread();

    // in worker thread
public Q_SLOTS:
    void loadRunnerMetaData();
    void loadRunner(int index);
    void setEnabledRunners(const QStringList &runnerIds);
    void startMatching();

    // in GUI thread
public:
    void launchDefaultMatches();
    bool launchQuery(const QString &query);
    void launchMoreMatches();
    int matchCount() const;
    QueryMatch matchAt(int index);

public Q_SLOTS:
    void syncMatches();

    // thread agnostic
public:
    QStringList enabledRunners() const;
    const QVector<RunnerMetaData> &runnerMetaData() const;
    QuerySession *session() const { return m_session; }
    void endQuerySession();
    QString query() const;
    bool setImageSize(const QSize &size);
    QSize imageSize() const;

Q_SIGNALS:
    void enabledRunnersChanged();
    void loadingRunnerMetaData();
    void loadedRunnerMetaData();
    void continueMatching();
    void busyChanged(int metaDataIndex);
    void runnerLoaded(int index);
    void resetModel();

public Q_SLOTS:
    void sessionDataRetrieved(const QUuid &sessionId, int, RunnerSessionData *data);

private Q_SLOTS:
    void updateBusyStatus();

private:
    // in GUI thread
    void startQuery(bool clearMatchers = true);

    // in worker thread
    bool startNextRunner();
    void retrieveSessionData(int index);

    // thread agnostic
    void clearSessionData();

    QThreadPool *m_threadPool;
    QuerySession *m_session;
    QStringList m_enabledRunnerIds;
    // these vectors are all the same size at all times
    QVector<RunnerMetaData> m_runnerMetaData;
    QVector<Runner *> m_runners;
    QVector<QSharedPointer<RunnerSessionData> > m_sessionData;
    QVector<MatchRunnable *> m_matchers;

    QSharedPointer<RunnerSessionData> m_dummySessionData;

    QReadWriteLock m_matchIndexLock;
    int m_runnerBookmark;
    int m_currentRunner;
    QueryContext m_context;
    QUuid m_sessionId;
    QTimer *m_restartMatchingTimer;
    int m_matchCount;

    QPointer<SessionDataThread> m_sessionDataThread;
};

class SessionDataRetriever : public QObject, public QRunnable
{
    Q_OBJECT
public:
    SessionDataRetriever(QThread *destinationThread, const QUuid &sessionId, int index, Runner *runner);
    void run();

Q_SIGNALS:
    void sessionDataRetrieved(const QUuid &sessionId, int index, RunnerSessionData *data);

private:
    QThread *m_destinationThread;
    Runner *m_runner;
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
    void finished(const Sprinter::QueryMatch &match, bool success);

private:
    QueryMatch m_match;
};

} // namespace

#endif