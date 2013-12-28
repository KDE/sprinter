#ifndef RUNNERMANAGERTHREAD
#define RUNNERMANAGERTHREAD

#include <QHash>
#include <QRunnable>
#include <QSet>
#include <QThread>
#include <QWeakPointer>

#include "runnercontext.h"

class AbstractRunner;
class RunnableMatch;
class RunnerManager;
class RunnerSessionData;

class RunnerManagerThread : public QThread
{
    Q_OBJECT

public:
    RunnerManagerThread(RunnerManager *parent = 0);
    ~RunnerManagerThread();

    void run();

Q_SIGNALS:
    void matchesUpdated();

public Q_SLOTS:
    void sessionDataRetrieved(AbstractRunner *runner, RunnerSessionData *data);
    void startQuery(const QString &query);
    void querySessionCompleted();

private:
    void loadRunners();
    void retrieveSessionData();
    void launchJobs();

    RunnerManager *m_manager;
    QSet<AbstractRunner *> m_runners;
    QHash<AbstractRunner *, RunnerSessionData *> m_runnerSessions;
    QHash<AbstractRunner *, RunnableMatch *> m_matchers;
    RunnerContext m_context;
    QString m_query;
};

class SessionDataRetriever : public QObject, public QRunnable
{
    Q_OBJECT
public:
    SessionDataRetriever(AbstractRunner *runner);
    void run();

Q_SIGNALS:
    void sessionDataRetrieved(AbstractRunner *runner, RunnerSessionData *data);

private:
    QWeakPointer<AbstractRunner> m_runner;
};

#endif

