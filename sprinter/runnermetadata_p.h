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

#ifndef RUNNERMETADATA
#define RUNNERMETADATA

#include "sprinter/querysession.h"

namespace Sprinter
{

class Runner;

struct RunnerMetaData
{
    RunnerMetaData()
        : generatesDefaultMatches(false),
          loaded(false),
          busy(false),
          fetchedSessionData(false)
    {
    }

    QString library;
    QString id;
    QString name;
    QString description;
    QString license;
    QString author;
    QString contactEmail;
    QString version;
    QString icon;
    QVector<QuerySession::MatchSource> sourcesUsed;
    QVector<QuerySession::MatchType> matchTypesGenerated;
    bool generatesDefaultMatches;
    bool loaded;
    bool busy;
    bool fetchedSessionData;
};

} // namespace

#endif