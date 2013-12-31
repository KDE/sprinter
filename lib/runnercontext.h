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

#ifndef RUNNERCONTEXT
#define RUNNERCONTEXT

#include <querymatch.h>

#include <QExplicitlySharedDataPointer>
#include <QString>

class QueryMatch;


class RunnerContext
{
public:
    RunnerContext();
    RunnerContext(const RunnerContext &other);
    ~RunnerContext();

    RunnerContext &operator=(const RunnerContext &other);

    void setQuery(const QString &query);
    QString query() const;

    bool isValid() const;
    void reset();

private:
    class Private;
    QExplicitlySharedDataPointer<Private> d;
};

#endif
