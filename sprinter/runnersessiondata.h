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

#include <sprinter/sprinter_export.h>
#include <sprinter/querymatch.h>

#include <QObject>
#include <QPointer>

namespace Sprinter
{

class Runner;
class QueryContext;
class QuerySession;

class SPRINTER_EXPORT RunnerSessionData : public QObject
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

    RunnerSessionData(Runner *runner);
    virtual ~RunnerSessionData();

    void setEnabled(bool enabled);
    bool enabled() const;

    Runner *runner() const;

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
    virtual bool shouldStartMatch(const QueryContext &context) const;

Q_SIGNALS:
    void busyChanged();

private:
    friend class QuerySessionThread;

    class Private;
    Private * const d;
};

} // namespace

#endif