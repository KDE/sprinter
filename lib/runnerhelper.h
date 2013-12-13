#ifndef RUNNERHELPER
#define RUNNERHELPER

#include <QObject>
#include <QHash>
#include <QRunnable>
#include <QSet>
#include <QWeakPointer>

#include "abstractrunner.h"

class RunnerHelper : public QObject
{
    Q_OBJECT

public:
    RunnerHelper(QObject *parent = 0);
    ~RunnerHelper();

    void loadRunners();

public Q_SLOTS:
    void sessionDataRetrieved(AbstractRunner *runner, RunnerSessionData *data);
    void startQuery(const QString &query);
    void querySessionCompleted();

private:
    void retrieveSessionData();
    void launchJobs();

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

