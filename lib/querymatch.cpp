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

#include "querymatch.h"

#include "abstractrunner.h"

class QueryMatch::Private : public QSharedData
{
public:
    Private(AbstractRunner *r)
        : runner(r),
          type(UnknownType),
          precision(UnrelatedMatch),
          updateInterval(0)
    {
    }

    AbstractRunner *runner;
    QString title;
    QString text;
    QString id;
    Type type;
    Precision precision;
    uint updateInterval;
};

QueryMatch::QueryMatch()
    : d(new Private(0))
{
}

//FIXME: if the runner is deleted?
QueryMatch::QueryMatch(AbstractRunner *runner)
    : d(new Private(runner))
{
}

QueryMatch::QueryMatch(const QueryMatch &other)
    : d(other.d)
{
}

QueryMatch::~QueryMatch()
{
}

QueryMatch &QueryMatch::operator=(const QueryMatch &other)
{
    d = other.d;
    return *this;
}

//FIXME: if the runner is deleted?
void QueryMatch::setTitle(const QString &title)
{
    d->title = title;
}

QString QueryMatch::title() const
{
    return d->title;
}

void QueryMatch::setText(const QString &text)
{
    d->text = text;
}

QString QueryMatch::text() const
{
    return d->text;
}

void QueryMatch::setType(Type type)
{
    d->type = type;
}

QueryMatch::Type QueryMatch::type() const
{
    return d->type;
}

void QueryMatch::setPrecision(Precision precision)
{
    d->precision = precision;
}

QueryMatch::Precision QueryMatch::precision() const
{
    return d->precision;
}

void QueryMatch::setInternalId(const QString &id)
{
    d->id = id;
}

QString QueryMatch::internalId() const
{
    return d->id;
}

void QueryMatch::setUpdateInterval(uint interval)
{
    d->updateInterval = interval;
    //TODO: implement interval checking
    if (interval > 0) {

    } else {

    }
}

uint QueryMatch::updateInterval() const
{
    return d->updateInterval;
}
