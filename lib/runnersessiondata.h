/*
 * Copyright (C) 2013 Aaron Seigo <aseigo@kde.org>
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

#include <QVector>

#include "querymatch.h"

class AbstractRunner;

//TODO: use QExplicitlySharedDataPointer or QSharedDataPointer instead of doing ref counting manually?
class RunnerSessionData
{
public:
    enum MatchState {
        SynchronizedMatches,
        NewMatches
    };

    RunnerSessionData(AbstractRunner *runner);
    virtual ~RunnerSessionData();

    AbstractRunner *runner() const;

    void addMatches(const QVector<QueryMatch> &matches);
    QVector<QueryMatch> matches(MatchState state) const;

    void ref();
    void deref();

private:
    class Private;
    Private * const d;
};

#endif