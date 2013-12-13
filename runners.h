#ifndef RUNNERS
#define RUNNERS


#include "abstractrunner.h"

class RunnerA : public AbstractRunner
{
    Q_OBJECT

public:
    RunnerA(QObject *parent = 0);

    RunnerSessionData *createSessionData();
    RunnableMatch *createMatcher(RunnerSessionData *sessionData, RunnerContext &context);
};

class RunnerB : public AbstractRunner
{
    Q_OBJECT

public:
    RunnerB(QObject *parent = 0);
    RunnableMatch *createMatcher(RunnerSessionData *sessionData, RunnerContext &context);
};

class RunnerCSessionData : public RunnerSessionData
{
public:
    QString data;
};

class RunnerC : public AbstractRunner
{
    Q_OBJECT

public:
    RunnerC(QObject *parent = 0);

    RunnerSessionData *createSessionData();
    RunnableMatch *createMatcher(RunnerSessionData *sessionData, RunnerContext &context);
};

class RunnerCMatcher : public RunnableMatch
{
public:
    RunnerCMatcher(RunnerC *runner, RunnerSessionData *sessionData, const RunnerContext &context);
    void match();

private:
    RunnerC *m_runner;
};

class RunnerD : public AbstractRunner
{
    Q_OBJECT

public:
    RunnerD(QObject *parent = 0);

    RunnerSessionData *createSessionData();
    RunnableMatch *createMatcher(RunnerSessionData *sessionData, RunnerContext &context);
};

#endif

