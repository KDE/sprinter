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

#include "runner.h"
#include "querycontext.h"
#include "querysession.h"
#include "querysession_p.h"
#include "querysessionthread_p.h"

// #define DEBUG_SYNC
// #define DEBUG_UPDATEMATCHES
namespace Sprinter
{

RunnerSessionData::Busy::Busy(RunnerSessionData *data)
    : m_data(data)
{
    Q_ASSERT(m_data);
    bool busy = m_data->d->busyCount.load() > 0;
    if (busy != m_data->d->busyCount.ref()) {
        emit m_data->busyChanged(!busy);
    }
}

RunnerSessionData::Busy::~Busy()
{
    if (m_data) {
        bool busy = m_data->d->busyCount.load() > 0;
        if (busy != m_data->d->busyCount.deref()) {
            emit m_data->busyChanged(!busy);
        }
    }
}

RunnerSessionData::RunnerSessionData(Runner *runner)
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

Runner *RunnerSessionData::runner() const
{
    return d->runner;
}

bool RunnerSessionData::shouldStartMatch(const QueryContext &context) const
{
    // no runner or not enabled -> return immediately
    if (!d->runner || !d->enabled) {
        return false;
    }

    // not if the query is too short return, unless
    // the runner does default matches and the context is a
    // request for default matches
    if (!(d->runner->generatesDefaultMatches() &&
          context.isDefaultMatchesRequest()) &&
        (uint)context.query().length() < d->runner->minQueryLength()) {
        return false;
    }

    // check if this runner requires network and if so skip matching
    // if there is no network available to us
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
        // we will set the matches to nothing unless this is a request
        // for more matches, in which case we just leave whatever we
        // currently have
        if (!context.fetchMore()) {
            setMatches(QVector<QueryMatch>(), context);
        }

        return;
    }

    auto updatePaging = [&]() {
            if (context.fetchMore()) {
                QMutexLocker lock(&d->currentMatchesLock);
                // this is the minimum number of matches we need to already
                // have to care about getting more
                const uint minSize = d->matchOffset + d->pageSize;

//                 qDebug() << "*****" << minSize << d->currentMatches.size() << d->syncedMatches.size();
                if (d->currentMatches.isEmpty()) {
                    if ((uint)d->syncedMatches.size() < minSize) {
                        return false;
                    } else {
                        d->matchOffset = minSize;
                    }
                } else if ((uint)d->currentMatches.size() < minSize) {
                    return false;
                } else {
                    d->matchOffset = minSize;
                }
//                 qDebug() << "***** WIN (min, cur, synced)" << minSize << d->currentMatches.size() << d->syncedMatches.size();

            } else {
                d->matchOffset = 0;
            }
            return true;
    };

    // reset if we can fetch more matches; the runner must
    // set this again after each match
    d->canFetchMoreMatches = false;

    // now we set up the paging
    if (!context.ifValid(updatePaging)) {
        return;
    }

    d->runner->match(this, context);
}

void RunnerSessionData::setMatches(const QVector<QueryMatch> &matches, const QueryContext &context)
{
    if (!context.isValid()) {
        return;
    }

#ifdef DEBUG_SYNC
    qDebug() << "New matches from, to: " << d->currentMatches.size() << matches.size();
    int count = 0;
    foreach (const QueryMatch &match, matches) {
        qDebug() << "     " << count++ << match.title();
    }
#endif

    {
        QMutexLocker lock(&d->currentMatchesLock);

        d->lastReceivedMatchOffset = d->matchOffset;

        if (matches.isEmpty()) {
            if ((uint)d->syncedMatches.size() <= d->lastReceivedMatchOffset) {
                // nothing going on here; we have not matches and
                // the syncedMatch set is smaller than the
                // size it will end up with matches removed already,
                // so we have nothing to remove
                return;
            }
        } else {
            d->currentMatches += matches;
        }

        d->matchesUnsynced = true;
    }

    if (d->session) {
        d->session->d->matchesArrived();
    }
}

void RunnerSessionData::updateMatches(const QVector<QueryMatch> &matches)
{
    Q_ASSERT(d->session);

    // TODO: find a nicer way to to do updates than comparing data(), nested loops..
#ifdef DEBUG_UPDATEMATCHES
    qDebug() << "updating" << matches.size();
#endif
    QMutexLocker lock(&d->currentMatchesLock);

    foreach (const QueryMatch &match, matches) {
#ifdef DEBUG_UPDATEMATCHES
        qDebug() << "looking for match of" << match.data();
#endif
        if (match.data().isNull()) {
            continue;
        }

        bool found = false;
        for (int i = 0; i < d->currentMatches.size(); ++i) {
            if (match.data() == d->currentMatches[i].data()) {
#ifdef DEBUG_UPDATEMATCHES
                qDebug() << "found update in pending matches at" << i << d->currentMatches[i].data();
#endif
                d->currentMatches[i] = match;
                found = true;
                break;
            }
#ifdef DEBUG_UPDATEMATCHES
            qDebug() << "compared" << i << match.data() << d->currentMatches[i].data();
#endif
        }

        if (found) {
            continue;
        }

        for (int i = 0; i < d->syncedMatches.size(); ++i) {
            if (match.data() == d->syncedMatches[i].data()) {
#ifdef DEBUG_UPDATEMATCHES
                qDebug() << "found update in existing matches at" << i << d->syncedMatches[i].data();
#endif
                d->syncedMatches[i] = match;
                d->updatedMatchIndexes.insert(i);
                d->session->d->matchesArrived();
                break;
            }
#ifdef DEBUG_UPDATEMATCHES
            qDebug() << "compared" << i << match.data() << d->syncedMatches[i].data();
#endif
        }
    }
}

QVector<QueryMatch> RunnerSessionData::matches(MatchState state) const
{
    QMutexLocker lock(&d->currentMatchesLock);
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

uint RunnerSessionData::resultsOffset() const
{
    return d->matchOffset;
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

void RunnerSessionData::Private::associateSession(QuerySession *newSession)
{
    if (newSession == session) {
        return;
    }

    session = newSession;
    if (session && !currentMatches.isEmpty()) {
        session->d->matchesArrived();
    }
}

int RunnerSessionData::Private::syncMatches(int modelOffset)
{
    Q_ASSERT(session);

    QVector<QueryMatch> unsynced;

    QMutexLocker lock(&currentMatchesLock);
    if (!updatedMatchIndexes.isEmpty()) {
        foreach (int index, updatedMatchIndexes) {
            if (index < syncedMatches.size()) {
#ifdef DEBUG_UPDATEMATCHES
                qDebug() << "Telling the model we've updated" << modelOffset + index;
#endif
                session->d->matchesUpdated(modelOffset + index, modelOffset + index);
            }
        }

        updatedMatchIndexes.clear();
    }

    lastSyncedMatchOffset = lastReceivedMatchOffset;

    if (matchesUnsynced) {
        matchesUnsynced = false;
        unsynced = currentMatches;
        currentMatches.clear();
    } else {
        return syncedMatches.size();
    }

#ifdef DEBUG_SYNC
    qDebug() << "SYNC model offset, synced, unsynced:" << modelOffset << syncedMatches.size() << unsynced.size();
#endif

    // only accept pagesize matches
    // may happen innocently when the page size changes between match and sync
    if ((uint)unsynced.size() > pageSize) {
        canFetchMoreMatches = true;
        unsynced.resize(pageSize);
    }

    if (syncedMatches.isEmpty()) {
        // no sync'd matches, so we only need to do something if we now do have matches
        if (!unsynced.isEmpty()) {
            // we had no matches, now we do
            session->d->addingMatches(modelOffset, modelOffset + unsynced.size());
            syncedMatches = unsynced;
            session->d->matchesAdded();
        }
    } else if (unsynced.isEmpty()) {
        // we had matches, and now we don't
        if ((uint)syncedMatches.size() > lastSyncedMatchOffset) {
            session->d->removingMatches(modelOffset + lastSyncedMatchOffset, modelOffset + syncedMatches.size() - lastSyncedMatchOffset);
            syncedMatches.resize(lastSyncedMatchOffset);
            session->d->matchesRemoved();
        }
    } else {
        // now the more complex situation: we have both synced and new matches
        // these need to be merged with the correct add/remove/update rows
        // methods called in the session (the model)
        const uint oldCount = syncedMatches.size() - lastSyncedMatchOffset;
        const uint newCount = unsynced.size();
        if (oldCount == newCount) {
            for (uint i = 0; i < newCount; ++i) {
                syncedMatches[i + lastSyncedMatchOffset] = unsynced[i];
            }

            session->d->matchesUpdated(modelOffset + lastSyncedMatchOffset,
                                       modelOffset + lastSyncedMatchOffset + newCount);
        } else if (oldCount < newCount) {
            session->d->addingMatches(modelOffset + syncedMatches.size(),
                                      modelOffset + syncedMatches.size() +
                                      (newCount - oldCount));
//             qDebug() << "was" << syncedMatches.size() << "will be"
//                      << (lastSyncedMatchOffset + newCount)
//                      << "has" << newCount << unsynced.size();
            syncedMatches.resize(lastSyncedMatchOffset + newCount);

            for (uint i = 0; i < newCount; ++i) {
                syncedMatches[i + lastSyncedMatchOffset] = unsynced[i];
            }
            session->d->matchesAdded();
            session->d->matchesUpdated(modelOffset + lastSyncedMatchOffset,
                                       modelOffset + lastSyncedMatchOffset + oldCount);
        } else {
            // more old matches than new
            session->d->removingMatches(modelOffset + lastSyncedMatchOffset + newCount,
                                        modelOffset + lastSyncedMatchOffset + oldCount);
            syncedMatches.resize(modelOffset + lastSyncedMatchOffset + newCount);
            for (uint i = 0; i < newCount; ++i) {
                syncedMatches[i + lastSyncedMatchOffset] = unsynced[i];
            }
            session->d->matchesRemoved();
            session->d->matchesUpdated(modelOffset + lastSyncedMatchOffset,
                                       modelOffset + lastSyncedMatchOffset + newCount);
        }
    }

    return syncedMatches.size();
}

} // namespace
#include "moc_runnersessiondata.cpp"
