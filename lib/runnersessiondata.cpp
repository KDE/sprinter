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

#include "runnersessiondata.h"

#include <QAtomicInt>
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>

#include "runnermanager.h"
#include "runnercontext.h"

#define DEBUG

class RunnerSessionData::Private
{
public:
    Private(AbstractRunner *r)
        : runner(r),
          manager(0),
          modelOffset(0),
          matchesUnsynced(false),
          pageSize(10),
          offset(0)
    {
    }

    AbstractRunner *runner;
    QAtomicInt ref;
    QVector<QueryMatch> syncedMatches;
    QVector<QueryMatch> currentMatches;
    RunnerManager *manager;
    QMutex currentMatchesLock;
    int modelOffset;
    bool matchesUnsynced;
    uint pageSize;
    uint offset;
};

RunnerSessionData::RunnerSessionData(AbstractRunner *runner)
    : QObject(0),
      d(new Private(runner))
{
}

RunnerSessionData::~RunnerSessionData()
{
}

void RunnerSessionData::associateManager(RunnerManager *manager)
{
    if (manager == d->manager) {
        return;
    }

    d->manager = manager;
    if (d->manager && !d->currentMatches.isEmpty()) {
        d->manager->matchesArrived();
    }
}

int RunnerSessionData::syncMatches(int offset)
{
    Q_ASSERT(d->manager);

    QVector<QueryMatch> unsynced;
    // TODO: do we need to syncronize this variable (threading)?
    d->modelOffset = offset;

    {
        QMutexLocker lock(&d->currentMatchesLock);
        if (d->matchesUnsynced) {
            d->matchesUnsynced = false;
            unsynced = d->currentMatches;
            d->currentMatches.clear();
        } else {
            return d->syncedMatches.count();
        }
    }

    if (d->syncedMatches.isEmpty()) {
        // no sync'd matches, so we only need to do something if we now do have matches
        if (!unsynced.isEmpty()) {
            // we had no matches, now we do
            d->manager->addingMatches(offset, offset + unsynced.size());
            d->syncedMatches = unsynced;
            d->manager->matchesAdded();
        }
    } else if (unsynced.isEmpty()) {
        // we had matches, and now we don't
        d->manager->removingMatches(offset, offset + d->syncedMatches.size());
        d->syncedMatches.clear();
        d->manager->matchesRemoved();
    } else {
        // now the more complex situation: we have both synced and new matches
        // these need to be merged with the correct add/remove/update rows
        // methods called in the manager (the model)
        //TODO: implement merging; this implementation is naive and does not
        // allow for updating results
        const int oldCount = d->syncedMatches.count();
        const int newCount = unsynced.count();
        if (oldCount == newCount) {
            d->syncedMatches = unsynced;
            d->manager->matchesUpdated(offset, offset + newCount);
        } else if (oldCount < newCount) {
            d->manager->addingMatches(offset + oldCount, offset + newCount);
            d->syncedMatches = unsynced;
            d->manager->matchesAdded();
            d->manager->matchesUpdated(offset, offset + newCount);
        } else {
            d->manager->removingMatches(offset + newCount, offset + oldCount);
            d->syncedMatches = unsynced;
            d->manager->matchesAdded();
            d->manager->matchesUpdated(offset, offset + newCount);
        }
    }

    return d->syncedMatches.count();
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

void RunnerSessionData::setMatches(const QVector<QueryMatch> &matches, const RunnerContext &context)
{
    if (!context.isValid()) {
        return;
    }

#ifdef DEBUG
    qDebug() << "New matches: " << matches.count();
    int count = 0;
    foreach (const QueryMatch &match, matches) {
        qDebug() << "     " << count++ << match.title();
    }
#endif

    {
        QMutexLocker lock(&d->currentMatchesLock);

        if (matches.isEmpty()) {
            if (d->currentMatches.isEmpty()) {
                // nothing going on here
                return;
            }

            d->currentMatches.clear();
        } else {
            //FIXME: implement *merging* rather than simply addition
            d->currentMatches += matches;
        }

        d->matchesUnsynced = true;
    }

    if (d->manager) {
        d->manager->matchesArrived();
    }
}

QVector<QueryMatch> RunnerSessionData::matches(MatchState state) const
{
    if (state == SynchronizedMatches) {
        return d->syncedMatches;
    } else {
        return d->currentMatches;
    }
}

void RunnerSessionData::setResultsPageSize(uint pageSize)
{
    d->pageSize = pageSize;
}

uint RunnerSessionData::resultsPageSize() const
{
    return d->pageSize;
}

void RunnerSessionData::setResultsOffset(uint offset)
{
    d->offset = offset;
}

uint RunnerSessionData::resultsOffset() const
{
    return d->offset;
}

#include "moc_runnersessiondata.cpp"
