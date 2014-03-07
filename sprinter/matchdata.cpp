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

#include "matchdata.h"

#include <QDebug>

#include "sprinter/querycontext.h"
#include "sprinter/runnersessiondata.h"

namespace Sprinter
{

class MatchData::Private
{
public:
    QPointer<Sprinter::RunnerSessionData> sessionData;
    Sprinter::QueryContext context;
    QVector<Sprinter::QueryMatch> matches;
    bool async;
};

MatchData::MatchData(RunnerSessionData *sessionData,
                     const QueryContext &context)
    : d(new Private)
{
    qDebug() << "Our session data object is" << sessionData;
    d->sessionData = sessionData;
    d->context = context;
    d->async = false;
}

MatchData::~MatchData()
{
    // if we still have a sessiondata object, and we either have
    // matches or this is a synchronous matcher, set the matches
    qDebug() << "maybe we'll sync up our data, huh?";
    if (d->sessionData && (!d->matches.isEmpty() || !d->async)) {
        qDebug() << "Our session data object is" << d->sessionData;
        qDebug() << "and how many matches?" << d->matches.count();
        d->sessionData->setMatches(d->matches, d->context);
    }

    delete d;
}

Sprinter::RunnerSessionData *MatchData::sessionData() const
{
//     qDebug() << "Sending out our session data object as" << d->sessionData;
    return d->sessionData;
}

Sprinter::QueryContext MatchData::queryContext() const
{
//     qDebug() << this << "query context ... " << d->context.isValid();
    return d->context;
}

bool MatchData::isValid() const
{
    return d->sessionData && d->context.isValid(d->sessionData);
}

void MatchData::setAsynchronous(bool async)
{
    d->async = async;
}

bool MatchData::isAsynchronous() const
{
    return d->async;
}

uint MatchData::matchCount() const
{
    return d->matches.size();
}

void MatchData::clearMatches()
{
    d->matches.clear();
}

MatchData &MatchData::operator<<(const Sprinter::QueryMatch &match)
{
    d->matches << match;
    return *this;
}

MatchData &MatchData::operator<<(const QVector<Sprinter::QueryMatch> &matches)
{
    d->matches << matches;
    return *this;
}

} //namspace