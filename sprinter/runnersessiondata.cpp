/*
 * Copyright (C) 2014 Aaron Seigo <aseigo@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "runnersessiondata.h"
#include "runnersessiondata_p.h"

#include <QDebug>

#include "runner.h"
#include "querycontext.h"
#include "querymatch_p.h"
#include "querysession.h"
#include "querysession_p.h"
#include "querysessionthread_p.h"

// #define DEBUG_SYNC
// #define DEBUG_UPDATEMATCHES
// #define DEBUG_REMOVEMATCHES
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
    delete d;
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

    // if we are fetching more results, make sure that the runner
    // can actually do so
    if (context.fetchMore() && !d->canFetchMoreMatches) {
         return false;
    }

    return context.isValid(this);
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
    if (!context.ifValid(updatePaging, this)) {
        return;
    }


    RunnerSessionData::Busy busy(this);
    MatchData matchData(this, context);
    d->runner->match(matchData);
}

void RunnerSessionData::setMatches(const QVector<QueryMatch> &matches, const QueryContext &context)
{
    if (!context.isValid(this)) {
        return;
    }

#ifdef DEBUG_SYNC
    qDebug() << this << "New matches from, to: " << d->currentMatches.size() << matches.size();
    for (int i = 0; i < matches.count(); ++i) {
        qDebug() << "     " << i << matches[i].title();
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
            for (int i = 0; i < matches.count(); ++i) {
                matches[i].d->sessionData = this;
            }
            d->currentMatches = matches;
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

#ifdef DEBUG_UPDATEMATCHES
    qDebug() << "updating" << matches.size();
#endif
    QMutexLocker lock(&d->currentMatchesLock);

    for (auto const &match: matches) {
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
                d->syncedMatches[i].d->sessionData = this;
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

void RunnerSessionData::removeMatches(const QVector<QueryMatch> &matches)
{
    Q_ASSERT(d->session);

#ifdef DEBUG_REMOVEMATCHES
    qDebug() << "removing" << matches.size();
#endif
    QMutexLocker lock(&d->currentMatchesLock);

    for (auto const &match: matches) {
#ifdef DEBUG_REMOVEMATCHES
        qDebug() << "looking for match of" << match.data();
#endif
        if (match.data().isNull()) {
            continue;
        }

        bool found = false;
        for (int i = 0; i < d->currentMatches.size(); ++i) {
            if (match.data() == d->currentMatches[i].data()) {
#ifdef DEBUG_REMOVEMATCHES
                qDebug() << "remove match in pending matches at" << i << d->currentMatches[i].data();
#endif
                d->currentMatches.removeAt(i);
                found = true;
                break;
            }
#ifdef DEBUG_REMOVEMATCHES
            qDebug() << "compared" << i << match.data() << d->currentMatches[i].data();
#endif
        }

        if (found) {
            continue;
        }

        for (int i = 0; i < d->syncedMatches.size(); ++i) {
            if (match.data() == d->syncedMatches[i].data()) {
#ifdef DEBUG_REMOVEMATCHES
                qDebug() << "remove match in existing matches at" << i << d->syncedMatches[i].data();
#endif
                d->removedMatchIndexes.insert(i);
                d->session->d->matchesArrived();
                break;
            }
#ifdef DEBUG_REMOVEMATCHES
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
    }, this);
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
        for (auto const &index: updatedMatchIndexes) {
            if (index < syncedMatches.size()) {
#ifdef DEBUG_UPDATEMATCHES
                qDebug() << "Telling the model we've updated" << modelOffset + index;
#endif
                session->d->matchesUpdated(modelOffset + index, modelOffset + index);
            }
        }

        updatedMatchIndexes.clear();
    }

    if (!removedMatchIndexes.isEmpty()) {
        for (auto const &index: removedMatchIndexes) {
            if (index < syncedMatches.size()) {
#ifdef DEBUG_REMOVEMATCHES
                qDebug() << "Telling the model we've removed" << modelOffset + index;
#endif
                session->d->removingMatches(modelOffset + index, modelOffset + index);
                syncedMatches.removeAt(index);
                session->d->matchesRemoved();
            }
        }

        removedMatchIndexes.clear();
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
#ifdef DEBUG_SYNC
        qDebug() << "HAD MATCHESS .. NOW WE DON'T? synced/lastOffset" << syncedMatches.size() << lastSyncedMatchOffset;
#endif
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
