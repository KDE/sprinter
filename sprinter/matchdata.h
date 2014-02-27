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

/**
 * @class MatchData
 * This class gets passed into @see Runner::match and does three primary things:
 * 1. Provides access to the RunnerSessionData object
 * 2. Provies acces to the QueryContext object
 * 3. Provides a place to store QueryMatch objects as they are generated
 *
 * The class will take care of adding matches to the RunnerSessionData object
 * automatically, relieving the Runner of having to do this.
 *
 * Objects of this type may not be copied or assigned to.
 */

class MatchData
{
public:
    /**
     * @param sessionData The RunnerSessionData object associated with the runner
     *                    the MatchData will be passed in to
     * @param context The current query's context, e.g. the query text, image size, etc.
     */
    MatchData(RunnerSessionData *sessionData,
                const QueryContext &context);
    ~MatchData();

    /**
     * @return The RunnerSessionData object associated with the runner
     * the MatchData will be passed in to
     */
    Sprinter::RunnerSessionData *sessionData() const;

    /**
     * @return The current query's context, e.g. the query text, image size, etc.
     */
    Sprinter::QueryContext queryContext() const;

    /**
     * @return true if this MatchData is still valid; this implies that there is both
     * valid QueryContext and a RunnerSessionData object
     */
    bool isValid() const;

    /**
     * If performing asynchronous matching which will possibly add matches to the
     * the set of QueryMatches after Runner::match has returned, the Runner must
     * call this method with async set to true. The default is false.
     * @param async true if matches may be added after Runner::match returns
     */
    void setAsynchronous(bool async);

    /**
     * @return true if matches may be added after Runner::match returns
     */
    bool isAsynchronous() const;

    /**
     * Used to add matches to the MatchData object
     */
    MatchData &operator<<(const Sprinter::QueryMatch &match);

    /**
     * Used to add matches to the MatchData object
     */
    MatchData &operator<<(const QVector<Sprinter::QueryMatch> &matches);

private:
    MatchData(const MatchData &other);
    MatchData &operator=(const MatchData &other);

    class Private;
    Private * const d;
};

} //namespace