#include "abstractrunner.h"

#include <QDebug>
#include <QSharedData>
#include <QWeakPointer>

class RunnerSessionData::Private
{
public:
    Private(AbstractRunner *r)
        : runner(r)
    {
    }

    AbstractRunner *runner;
    QAtomicInt ref;
    QVector<QueryMatch> syncedMatches;
    QVector<QueryMatch> currentMatches;
};

RunnerSessionData::RunnerSessionData(AbstractRunner *runner)
    : d(new Private(runner))
{
}

RunnerSessionData::~RunnerSessionData()
{
}

void RunnerSessionData::ref()
{
    d->ref.ref();
}

void RunnerSessionData::deref()
{
    if (!d->ref.deref()) {
        delete this;
    }
}

void RunnerSessionData::addMatches(const QVector<QueryMatch> &matches)
{
//     if (!d->q) {
//         return;
//     }
    //FIXME: implement *merging* rather than simply addition
    //FIXME: notify the outside world that matches have changed
    d->currentMatches += matches;
    qDebug() << "Got new matches ..." << matches.count();
}

QVector<QueryMatch> RunnerSessionData::matches(MatchState state) const
{
    if (state == SynchronizedMatches) {
        return d->syncedMatches;
    } else {
        return d->currentMatches;
    }
}

class AbstractRunner::Private
{
public:
    Private()
        : minQueryLength(3)
    {
    }

    uint minQueryLength;
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

void AbstractRunner::setMinQueryLength(uint length)
{
    d->minQueryLength = length;
}

int AbstractRunner::minQueryLength() const
{
    return d->minQueryLength;
}

RunnerSessionData *AbstractRunner::createSessionData()
{
    return new RunnerSessionData(this);
}

bool AbstractRunner::shouldStartMatch(const RunnerSessionData *sessionData, const RunnerContext &context) const
{
    if (!sessionData) {
        return false;
    }

    if (!context.isValid()) {
        return false;
    }

    if ((uint)context.query().length() < d->minQueryLength) {
        return false;
    }

    return true;
}

void AbstractRunner::startMatch(RunnerSessionData *sessionData, RunnerContext &context)
{
    if (shouldStartMatch(sessionData, context)) {
        match(sessionData, context);
    }
}

void AbstractRunner::match(RunnerSessionData *sessionData, RunnerContext &context)
{
    Q_UNUSED(sessionData)
    Q_UNUSED(context)
}

#include "abstractrunner.moc"

