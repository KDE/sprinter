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

#ifndef QUERYMATCH_P_H
#define QUERYMATCH_P_H

#include <QPointer>
#include <QSharedData>

#include "querysession.h"

namespace Sprinter
{

class RunnerSessionData;

class QueryMatch::Private : public QSharedData
{
public:
    Private()
        : type(QuerySession::UnknownType),
          source(QuerySession::FromInternalSource),
          precision(QuerySession::UnrelatedMatch),
          isSearchTerm(false)
    {
    }

    QPointer<RunnerSessionData> sessionData;
    QString title;
    QString text;
    QuerySession::MatchType type;
    QuerySession::MatchSource source;
    QuerySession::MatchPrecision precision;
    QVariant data;
    QVariant userData;
    QImage image;
    bool isSearchTerm;
};

} //namespace
#endif