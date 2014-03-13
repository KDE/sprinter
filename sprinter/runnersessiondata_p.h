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

#ifndef RUNNERSESSIONDATA_PRIVATE_H
#define RUNNERSESSIONDATA_PRIVATE_H

#include <QAtomicInt>
#include <QMutex>
#include <QMutexLocker>
#include <QSet>
#include <QUuid>

namespace Sprinter
{

class RunnerSessionData::Private
{
public:
    Private(Runner *r)
        : runner(r),
          session(0),
          currentMatchesLock(QMutex::Recursive),
          matchesUnsynced(false),
          canFetchMoreMatches(false),
          enabled(false),
          pageSize(10),
          matchOffset(0),
          lastReceivedMatchOffset(0),
          lastSyncedMatchOffset(0)
    {
    }

    int syncMatches(int offset);
    void associateSession(QuerySession *session);

    Runner *runner;
    QAtomicInt busyCount;
    QVector<QueryMatch> syncedMatches;
    QVector<QueryMatch> currentMatches;
    QSet<int> updatedMatchIndexes;
    QuerySession *session;
    QMutex currentMatchesLock;
    QUuid sessionId;
    bool matchesUnsynced;
    bool canFetchMoreMatches;
    bool enabled;
    uint pageSize;
    uint matchOffset;
    uint lastReceivedMatchOffset;
    uint lastSyncedMatchOffset;
};

} // namespace

#endif
