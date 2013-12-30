#include "runnersessiondata.h"

#include <QDebug>
#include <QAtomicInt>

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

AbstractRunner *RunnerSessionData::runner() const
{
    return d->runner;
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
    if (matches.isEmpty()) {
        return;
    }

    qDebug() << "New matches: " << matches.count();
    int count = 0;
    foreach (const QueryMatch &match, matches) {
        qDebug() << "     " << count++ << match.title();
    }
    d->currentMatches += matches;
}

QVector<QueryMatch> RunnerSessionData::matches(MatchState state) const
{
    if (state == SynchronizedMatches) {
        return d->syncedMatches;
    } else {
        return d->currentMatches;
    }
}
