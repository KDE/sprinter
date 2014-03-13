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

#ifndef RUNNERMANAGER_PRIVATE
#define RUNNERMANAGER_PRIVATE

namespace Sprinter
{

class QuerySession;
class QueryMatch;
class QuerySessionThread;
class RunnerModel;
class NonRestartingTimer;

class QuerySession::Private
{
public:
    Private(QuerySession *manager);

    void addingMatches(int start, int end);
    void matchesAdded();
    void removingMatches(int start, int end);
    void matchesRemoved();
    void matchesUpdated(int start, int end);
    void matchesArrived();
    void resetModel();
    void executionFinished(const Sprinter::QueryMatch &match, bool success);
    void startMatchSynchronization();

    QuerySession *q;
    QThread *workerThread;
    QuerySessionThread *worker;
    RunnerModel *runnerModel;
    NonRestartingTimer *syncTimer;
    QHash<int, QByteArray> roles;
    QVector<int> roleColumns;
    QHash<int, QueryMatch> executingMatches;
    int imageRoleColumn;
    bool matchesArrivedWhileExecuting;
};

} // namespace
#endif