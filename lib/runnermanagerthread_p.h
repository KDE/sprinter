#ifndef RUNNERMANAGERTHREAD
#define RUNNERMANAGERTHREAD

#include <QReadWriteLock>
#include <QRunnable>
#include <QThread>
#include <QVector>
#include <QWeakPointer>
#include <QUuid>

#include "runnercontext.h"

class QTimer;

class AbstractRunner;
class RunnableMatch;
class RunnerManager;
class RunnerSessionData;

class MatchRunnable : public QRunnable
{
public:
    MatchRunnable(AbstractRunner *runner, RunnerSessionData *sessionData, RunnerContext &context);
    void run();

private:
    AbstractRunner *m_runner;
    RunnerSessionData *m_sessionData;
    RunnerContext &m_context;
};

class RunnerManagerThread : public QThread
{
    Q_OBJECT

public:
    RunnerManagerThread(RunnerManager *parent = 0);
    ~RunnerManagerThread();

    void run();


Q_SIGNALS:
    void requestFurtherMatching();

public Q_SLOTS:
    void sessionDataRetrieved(const QUuid &sessionId, int, RunnerSessionData *data);
    void startQuery(const QString &query);
    void querySessionCompleted();
    void startMatching();

private:
    void loadRunners();
    void retrieveSessionData();
    bool startNextRunner();

    RunnerManager *m_manager;
    QVector<AbstractRunner *> m_runners;
    QVector<RunnerSessionData *> m_sessionData;
    QVector<MatchRunnable *> m_matchers;
    QReadWriteLock m_matchIndexLock;
    int m_runnerBookmark;
    int m_currentRunner;
    RunnerContext m_context;
    QString m_query;
    QUuid m_sessionId;
    QTimer *m_restartMatchingTimer;
    MatchRunnable *m_dummyMatcher;
};

class SessionDataRetriever : public QObject, public QRunnable
{
    Q_OBJECT
public:
    SessionDataRetriever(const QUuid &sessionId, int index, AbstractRunner *runner);
    void run();

Q_SIGNALS:
    void sessionDataRetrieved(const QUuid &sessionId, int index, RunnerSessionData *data);

private:
    AbstractRunner *m_runner;
    QUuid m_sessionId;
    int m_index;
};

#endif

