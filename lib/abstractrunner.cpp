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

RunnableMatch::RunnableMatch(RunnerSessionData *sessionData, const RunnerContext &context)
    : d(new Private)
{
    d->sessionData = sessionData;
    d->context = context;
}

RunnableMatch::~RunnableMatch()
{
    delete d;
}

RunnerSessionData *RunnableMatch::sessionData()
{
    return d->sessionData;
}

void RunnableMatch::addMatches(const QList<QueryMatch> &matches)
{
    d->context.addMatches(matches);
}

const QString RunnableMatch::query() const
{
    return d->context.query();
}

bool RunnableMatch::isValid() const
{
    return d->context.isValid();
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

RunnableMatch *AbstractRunner::createMatcher(RunnerSessionData *sessionData, RunnerContext &context)
{
    Q_UNUSED(sessionData)
    Q_UNUSED(context)
    return 0;
}

#include "abstractrunner.moc"

