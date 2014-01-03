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

#ifndef RUNNERSESSIONDATA
#define RUNNERSESSIONDATA

#include <QObject>

#include "querymatch.h"

class AbstractRunner;
class RunnerContext;
class RunnerManager;

//TODO: use QExplicitlySharedDataPointer or QSharedDataPointer instead of doing ref counting manually?
class RunnerSessionData : public QObject
{
    Q_OBJECT

public:
    enum MatchState {
        SynchronizedMatches,
        PendingMatches
    };

    RunnerSessionData(AbstractRunner *runner);
    virtual ~RunnerSessionData();

    AbstractRunner *runner() const;
    QVector<QueryMatch> matches(MatchState state) const;

    void associateManager(RunnerManager *manager);

    void ref();
    void deref();

    virtual bool shouldStartMatch(const RunnerContext &context) const;


    void setMatches(const QVector<QueryMatch> &matches, const RunnerContext &context);
    void updateMatches(const QVector<QueryMatch> &matches);

    void setResultsPageSize(uint pageSize);
    uint resultsPageSize() const;

    void setResultsOffset(uint page);
    uint resultsOffset() const;

private:
    friend class RunnerManagerThread;
    int syncMatches(int offset);

    class Private;
    Private * const d;
};

#endif