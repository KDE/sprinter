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
#include <QPointer>

#include "querymatch.h"

class AbstractRunner;
class QueryContext;
class RunnerManager;

class RunnerSessionData : public QObject
{
    Q_OBJECT

public:
    enum MatchState {
        SynchronizedMatches,
        PendingMatches
    };

    class Busy {
        public:
            Busy(RunnerSessionData *sessionData);
            ~Busy();
        private:
            QPointer<RunnerSessionData> m_data;
    };

    RunnerSessionData(AbstractRunner *runner);
    virtual ~RunnerSessionData();

    void setEnabled(bool enabled);
    bool enabled() const;

    AbstractRunner *runner() const;

    void associateManager(RunnerManager *manager);

    virtual bool shouldStartMatch(const QueryContext &context) const;

    void setMatches(const QVector<QueryMatch> &matches, const QueryContext &context);
    void updateMatches(const QVector<QueryMatch> &matches);
    QVector<QueryMatch> matches(MatchState state) const;

    void setResultsPageSize(uint pageSize);
    uint resultsPageSize() const;

    void setResultsOffset(uint page);
    uint resultsOffset() const;

    bool isBusy() const;

    void setCanFetchMoreMatches(bool hasMore, const QueryContext &context);
    bool canFetchMoreMatches() const;

    void startMatch(const QueryContext &context);

Q_SIGNALS:
    void busyChanged();

private:
    friend class RunnerManagerThread;

    class Private;
    Private * const d;
};

#endif