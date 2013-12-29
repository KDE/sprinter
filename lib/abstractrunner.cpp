#include "abstractrunner.h"

#include <QDebug>
#include <QSharedData>
#include <QWeakPointer>

class RunnableMatch::Private
{
public:
    RunnerSessionData *sessionData;
    RunnerContext context;
};

RunnableMatch::RunnableMatch(RunnerSessionData *sessionData)
    : d(new Private)
{
    d->sessionData = sessionData;
}

RunnableMatch::~RunnableMatch()
{
    delete d;
}

RunnerSessionData *RunnableMatch::sessionData()
{
    return d->sessionData;
}

void RunnableMatch::setContext(const RunnerContext &context)
{
    d->context = context;
}

RunnerContext &RunnableMatch::context() const
{
    return d->context;
}

void RunnableMatch::run()
{
    if (d->context.isValid()) {
        match();
    }
}

class AbstractRunner::Private
{
public:

};

AbstractRunner::AbstractRunner(QObject *parent)
    : QObject(parent),
      d(new Private)
{
}

AbstractRunner::~AbstractRunner()
{
    delete d;
}

RunnerSessionData *AbstractRunner::createSessionData()
{
    return 0;
}

RunnableMatch *AbstractRunner::createMatcher(RunnerSessionData *sessionData)
{
    Q_UNUSED(sessionData)
    return 0;
}

#include "abstractrunner.moc"

