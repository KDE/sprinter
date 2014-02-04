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
#include "runnersessiondata_p.h"

#include <QDebug>

#include "abstractrunner.h"
#include "querysession.h"
#include "querysession_p.h"
#include "querycontext.h"

// #define DEBUG_SYNC

RunnerSessionData::Busy::Busy(RunnerSessionData *data)
    : m_data(data)
{
    Q_ASSERT(m_data);
    bool busy = m_data->d->busyCount.load() > 0;
    if (busy != m_data->d->busyCount.ref()) {
        emit m_data->busyChanged();
    }
}

RunnerSessionData::Busy::~Busy()
{
    if (m_data) {
        bool busy = m_data->d->busyCount.load() > 0;
        if (busy != m_data->d->busyCount.deref()) {
            emit m_data->busyChanged();
        }
    }
}

RunnerSessionData::RunnerSessionData(AbstractRunner *runner)
    : QObject(0),
      d(new Private(runner))
{
}

RunnerSessionData::~RunnerSessionData()
{
}

void RunnerSessionData::setEnabled(bool enabled)
{
    d->enabled = enabled;
}

bool RunnerSessionData::enabled() const
{
    return d->enabled;
}

AbstractRunner *RunnerSessionData::runner() const
{
    return d->runner;
}

void RunnerSessionData::associateManager(QuerySession *manager)
{
    if (manager == d->manager) {
        return;
    }

    d->manager = manager;
    if (d->manager && !d->currentMatches.isEmpty()) {
        d->manager->d->matchesArrived();
    }
}

bool RunnerSessionData::shouldStartMatch(const QueryContext &context) const
{
    // no runner or not enabled -> return immediately
    if (!d->runner || !d->enabled) {
        return false;
    }

    // if the query is too short return, unless
    // the runner does default matches and the context is a
    // request for default matches

    if (!(d->runner->generatesDefaultMatches() &&
          context.isDefaultMatchesRequest()) &&
        (uint)context.query().length() < d->runner->minQueryLength()) {
        return false;
    }

    // check if this runner requires network and if so deny it
    if (d->runner->sourcesUsed().size() == 1 &&
        d->runner->sourcesUsed()[0] == QuerySession::FromNetworkService &&
        !context.networkAccessible()) {
        return false;
    }

    /*
        TODO: should QueryContext have an optional matchTypes set which
        can be used to filter runners on a per-query/per-session basis?
        if (!context.matchTypesGenerated.isEmpty() &&
            !runner->matchTypesGenerated().isEmpty()) {

        }
    */

    // if we are fetching more results, make sure that the runner
    // can actually do so
    if (context.fetchMore() && !d->canFetchMoreMatches) {
         return false;
    }

    return context.isValid();
}

void RunnerSessionData::startMatch(const QueryContext &context)
{
    if (!shouldStartMatch(context)) {
        return;
    }

    auto updatePaging = [&]() {
            if (context.fetchMore()) {
                // this is the minimum number of matches we need to already
                // have to care about ggetting more
                const uint minSize = d->offset + d->pageSize;
                if (d->currentMatches.isEmpty()) {
                    if ((uint)d->syncedMatches.size() >= minSize) {
                        d->offset += d->pageSize;
                    }
                } else if ((uint)d->currentMatches.size() >= minSize) {
                    d->offset += d->pageSize;
                } else {
                    return false;
                }
            } else {
                d->offset = 0;
            }
            return true;
    };

    // now we set up the paging
    if (!context.ifValid(updatePaging)) {
        return;
    }

    // reset if we can fetch more matches; the runner must
    // set this again after each match
    d->canFetchMoreMatches = false;
    d->runner->match(this, context);
}

int RunnerSessionData::Private::syncMatches(int offset)
{
    Q_ASSERT(manager);

    QVector<QueryMatch> unsynced;

    {
        QMutexLocker lock(&currentMatchesLock);
        if (matchesUnsynced) {
            matchesUnsynced = false;
            unsynced = currentMatches;
            currentMatches.clear();
        } else {
            return syncedMatches.count();
        }
    }

#ifdef DEBUG_SYNC
    qDebug() << "SYNC offset, synced, unsynced:" << offset << syncedMatches.count() << unsynced.count();
#endif

    // only accept pagesize matches
    if ((uint)unsynced.size() > pageSize) {
        unsynced.resize(pageSize);
    }

    if (syncedMatches.isEmpty()) {
        // no sync'd matches, so we only need to do something if we now do have matches
        if (!unsynced.isEmpty()) {
            // we had no matches, now we do
            manager->d->addingMatches(offset, offset + unsynced.size());
            syncedMatches = unsynced;
            manager->d->matchesAdded();
        }
    } else if (unsynced.isEmpty()) {
        // we had matches, and now we don't
        manager->d->removingMatches(offset, offset + syncedMatches.size());
        syncedMatches.clear();
        manager->d->matchesRemoved();
    } else {
        // now the more complex situation: we have both synced and new matches
        // these need to be merged with the correct add/remove/update rows
        // methods called in the manager (the model)
        //TODO: implement merging; this implementation is naive and does not
        // allow for updating results
        const int oldCount = syncedMatches.count();
        const int newCount = unsynced.count();
        if (oldCount == newCount) {
            syncedMatches = unsynced;
            manager->d->matchesUpdated(offset, offset + newCount);
        } else if (oldCount < newCount) {
            manager->d->addingMatches(offset + oldCount, offset + newCount);
            syncedMatches = unsynced;
            manager->d->matchesAdded();
            manager->d->matchesUpdated(offset, offset + newCount);
        } else {
            manager->d->removingMatches(offset + newCount, offset + oldCount);
            syncedMatches = unsynced;
            manager->d->matchesAdded();
            manager->d->matchesUpdated(offset, offset + newCount);
        }
    }

    return syncedMatches.count();
}

void RunnerSessionData::setMatches(const QVector<QueryMatch> &matches, const QueryContext &context)
{
    if (!context.isValid()) {
        return;
    }

#ifdef DEBUG_SYNC
    qDebug() << "New matches from, to: " << d->currentMatches.count() << matches.count();
    int count = 0;
    foreach (const QueryMatch &match, matches) {
        qDebug() << "     " << count++ << match.title();
    }
#endif

    {
        QMutexLocker lock(&d->currentMatchesLock);

        if (matches.isEmpty()) {
            d->currentMatches.clear();

            if (d->syncedMatches.isEmpty()) {
                // nothing going on here
                return;
            }
        } else {
            //FIXME: implement *merging* rather than simply addition
            d->currentMatches += matches;
        }

        d->matchesUnsynced = true;
    }

    if (d->manager) {
        d->manager->d->matchesArrived();
    }
}

void RunnerSessionData::updateMatches(const QVector<QueryMatch> &matches)
{
    Q_ASSERT(d->manager);

    // FIXME: this is a truly horrible way of doing this: nested for loops,
    //        comparing data() .. *shudder*
    QMutexLocker lock(&d->currentMatchesLock);
    //qDebug() << "updating" << matches.count();
    bool updateModel = false;

    foreach (const QueryMatch &match, matches) {
        //qDebug() <<" looking through" << match.data();
        if (match.data().isNull()) {
            continue;
        }

        QMutableVectorIterator<QueryMatch> cit (d->currentMatches);
        int count = 0;
        bool found = false;

        while (cit.hasNext()) {
            if (match.data() == cit.next().data()) {
                //qDebug() << "found update in pending matches at" << count << cit.value().data();
                cit.setValue(match);
                d->matchesUnsynced = true;
                d->manager->d->matchesArrived();
                found = true;
                break;
            }
            //qDebug() << "compared" << match.data() << cit.value().data();
            ++count;
        }

        if (!found) {
            QVectorIterator<QueryMatch> sit (d->syncedMatches);
            count = 0;
            while (sit.hasNext()) {
                if (match.data() == sit.next().data()) {
                    //qDebug() << "found update in existing matches at" << count;
                    d->currentMatches << match;
                    d->matchesUnsynced = true;
                    found = true;
                    break;
                }
                ++count;
            }
        }

        updateModel = updateModel || found;
    }

    if (updateModel) {
        d->manager->d->matchesArrived();
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

bool RunnerSessionData::isBusy() const
{
    return d->busyCount.load() > 0;
}

void RunnerSessionData::setCanFetchMoreMatches(bool hasMore, const QueryContext &context)
{
    context.ifValid([&]() {
        d->canFetchMoreMatches = hasMore;
        return true;
    });
}

bool RunnerSessionData::canFetchMoreMatches() const
{
    return d->canFetchMoreMatches;
}

#include "moc_runnersessiondata.cpp"
