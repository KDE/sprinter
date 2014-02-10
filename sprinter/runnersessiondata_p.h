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

#ifndef RUNNERSESSIONDATA_PRIVATE_H
#define RUNNERSESSIONDATA_PRIVATE_H

#include <QAtomicInt>
#include <QMutex>
#include <QMutexLocker>
#include <QSet>

namespace Sprinter
{

class RunnerSessionData::Private
{
public:
    Private(Runner *r)
        : runner(r),
          session(0),
          matchesUnsynced(false),
          canFetchMoreMatches(false),
          enabled(false),
          pageSize(10),
          offset(0)
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
    bool matchesUnsynced;
    bool canFetchMoreMatches;
    bool enabled;
    uint pageSize;
    uint offset;
};

} // namespace

#endif
