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

#include "runner.h"

#include <QDebug>

#include "runner_p.h"
#include "runnersessiondata.h"

namespace Sprinter
{

Runner::Runner(QObject *parent)
    : QObject(parent),
      d(new Private)
{
}

Runner::~Runner()
{
    delete d;
}

QString Runner::id() const
{
    return d->id;
}

void Runner::setMinQueryLength(uint length)
{
    d->minQueryLength = length;
}

uint Runner::minQueryLength() const
{
    return d->minQueryLength;
}

bool Runner::generatesDefaultMatches() const
{
    return d->hasDefaultMatches;
}

void Runner::setGeneratesDefaultMatches(bool hasDefaultMatches)
{
    d->hasDefaultMatches = hasDefaultMatches;
}

RunnerSessionData *Runner::createSessionData()
{
    return new RunnerSessionData(this);
}

void Runner::match(MatchData &matchData)
{
    Q_UNUSED(matchData)
}

bool Runner::startExec(const QueryMatch &match)
{
    if (match.runner() != this) {
        return false;
    }

    return exec(match);
}

bool Runner::exec(const QueryMatch &match)
{
    return match.sendUserDataToClipboard();
}

QVector<QuerySession::MatchType> Runner::matchTypesGenerated() const
{
    return d->matchTypes;
}

void Runner::setMatchTypesGenerated(const QVector<QuerySession::MatchType> types)
{
    d->matchTypes = types;
}

QVector<QuerySession::MatchSource> Runner::sourcesUsed() const
{
    return d->matchSources;
}

void Runner::setSourcesUsed(const QVector<QuerySession::MatchSource> &sources)
{
    d->matchSources = sources;
}

QImage Runner::generateImage(const QIcon &icon, const Sprinter::QueryContext &context)
{
    QImage image = d->imageCache.value(icon.cacheKey());
    if (image.isNull() || image.size() != context.imageSize()) {
        image = icon.pixmap(context.imageSize()).toImage();
        d->imageCache.insert(icon.cacheKey(), image);
    }

    return image;
}

} // namespace

#include "moc_runner.cpp"
