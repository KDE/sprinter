
#include "runners.h"
#include <unistd.h>

#include <QDebug>

RunnerA::RunnerA(QObject *parent)
    : AbstractRunner(parent)
{
    sleep(3);

}


RunnerSessionData *RunnerA::createSessionData()
{
    return 0;
}

RunnableMatch *RunnerA::createMatcher(RunnerSessionData *sessionData, RunnerContext &context)
{
    return 0;
}


RunnerB::RunnerB(QObject *parent)
    : AbstractRunner(parent)
{
}

RunnableMatch *RunnerB::createMatcher(RunnerSessionData *sessionData, RunnerContext &context)
{
    return 0;
}

RunnerCMatcher::RunnerCMatcher(RunnerC *runner, RunnerSessionData *sessionData, const RunnerContext &context)
    : RunnableMatch(sessionData, context)
{
}

void RunnerCMatcher::match()
{
    RunnerCSessionData *session = static_cast<RunnerCSessionData *>(sessionData());

    if (query() == "plasma") {
        qDebug() << "Session data: " << (session ? session->data : "----") << "; query: " << query();
        QList<QueryMatch> matches;
        //FIXME: if the runner is deleted?
        QueryMatch match(m_runner);
        match.setText("Sucks");
        matches << match;
        addMatches(matches);
    }
}

RunnerC::RunnerC(QObject *parent)
    : AbstractRunner(parent)
{

}

RunnerSessionData *RunnerC::createSessionData()
{
    RunnerCSessionData *session = new RunnerCSessionData;
    session->data = "Testing";
    return session;
}

RunnableMatch *RunnerC::createMatcher(RunnerSessionData *sessionData, RunnerContext &context)
{
    RunnerCMatcher *matcher = new RunnerCMatcher(this, sessionData, context);
    return matcher;
}


RunnerD::RunnerD(QObject *parent)
    : AbstractRunner(parent)
{

}


RunnerSessionData *RunnerD::createSessionData()
{
    return 0;
}

RunnableMatch *RunnerD::createMatcher(RunnerSessionData *sessionData, RunnerContext &context)
{
    return 0;
}

#include "runners.moc"


