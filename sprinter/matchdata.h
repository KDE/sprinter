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

#include <sprinter/querymatch.h>

#include <QVector>

namespace Sprinter
{

class QueryContext;
class RunnerSessionData;

class MatchData
{
public:
    MatchData(RunnerSessionData *sessionData,
                const QueryContext &context);
    ~MatchData();

    Sprinter::RunnerSessionData *sessionData() const;
    Sprinter::QueryContext queryContext() const;

    bool isValid() const;

    void setAsynchronous(bool async);
    bool isAsynchronous() const;

    MatchData &operator<<(Sprinter::QueryMatch &match);
    MatchData &operator<<(QVector<Sprinter::QueryMatch> &matches);


private:
    MatchData(const MatchData &other);
    MatchData &operator=(const MatchData &other);

    class Private;
    Private * const d;
};

} //namespace