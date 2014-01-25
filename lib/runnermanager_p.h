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

#ifndef RUNNERMANAGER_PRIVATE
#define RUNNERMANAGER_PRIVATE

class RunnerManager;
class QueryMatch;
class RunnerManagerThread;
class RunnerModel;

class RunnerManager::Private
{
public:
    Private(RunnerManager *manager);

    void addingMatches(int start, int end);
    void matchesAdded();
    void removingMatches(int start, int end);
    void matchesRemoved();
    void matchesUpdated(int start, int end);
    void matchesArrived();
    void resetModel();
    void executionFinished(const QueryMatch &match, bool success);

    RunnerManager *q;
    RunnerManagerThread *thread;
    RunnerModel *runnerModel;
    QString query;
    QHash<int, QByteArray> roles;
    QVector<int> roleColumns;
    QVector<QPair<QueryMatch, int> > executingMatches;
    bool matchesArrivedWhileExecuting;
};

#endif
