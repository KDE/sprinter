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

#include "abstractrunner.h"

#include <QDebug>

#include "runnersessiondata.h"

class AbstractRunner::Private
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
};

AbstractRunner::AbstractRunner(QObject *parent)
    : QObject(parent),
      d(new Private)
{
}

AbstractRunner::~AbstractRunner()
{
    delete d;
}

QString AbstractRunner::id() const
{
    return d->id;
}

void AbstractRunner::setId(const QString &newId)
{
    // only let it be set once; not in the ctor due to the fun of
    // plugin loading
    if (d->id.isEmpty()) {
        d->id = newId;
    }
}

void AbstractRunner::setMinQueryLength(uint length)
{
    d->minQueryLength = length;
}

uint AbstractRunner::minQueryLength() const
{
    return d->minQueryLength;
}

bool AbstractRunner::generatesDefaultMatches() const
{
    return d->hasDefaultMatches;
}

void AbstractRunner::setGeneratesDefaultMatches(bool hasDefaultMatches)
{
    d->hasDefaultMatches = hasDefaultMatches;
}

RunnerSessionData *AbstractRunner::createSessionData()
{
    return new RunnerSessionData(this);
}

void AbstractRunner::match(RunnerSessionData *sessionData, const QueryContext &context)
{
    Q_UNUSED(sessionData)
    Q_UNUSED(context)
}

bool AbstractRunner::startExec(const QueryMatch &match)
{
    if (match.runner() != this) {
        return false;
    }

    return exec(match);
}

bool AbstractRunner::exec(const QueryMatch &match)
{
    return match.sendUserDataToClipboard();
}

QVector<QuerySession::MatchType> AbstractRunner::matchTypesGenerated() const
{
    return d->matchTypes;
}

void AbstractRunner::setMatchTypesGenerated(const QVector<QuerySession::MatchType> types)
{
    d->matchTypes = types;
}

QVector<QuerySession::MatchSource> AbstractRunner::sourcesUsed() const
{
    return d->matchSources;
}

void AbstractRunner::setSourcesUsed(const QVector<QuerySession::MatchSource> &sources)
{
    d->matchSources = sources;
}

#include "moc_abstractrunner.cpp"
