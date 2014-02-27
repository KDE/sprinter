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

#ifndef SPRINTER_ABSTRACTRUNNER_P_H
#define SPRINTER_ABSTRACTRUNNER_P_H

#include <QCache>

namespace Sprinter
{

class Runner::Private
{
public:
    Private()
        : minQueryLength(3),
          hasDefaultMatches(false)
    {
    }

    uint minQueryLength;
    QString id;
    bool hasDefaultMatches;
    QVector<QuerySession::MatchType> matchTypes;
    QVector<QuerySession::MatchSource> matchSources;
    static QCache<qint64, QImage> s_imageCache;
};

}

#endif