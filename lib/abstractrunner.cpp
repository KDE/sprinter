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

#include <QApplication>
#include <QClipboard>
#include <QDebug>

#include "runnersessiondata.h"

class AbstractRunner::Private
{
public:
    Private()
        : minQueryLength(3)
    {
    }

    uint minQueryLength;
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

void AbstractRunner::setMinQueryLength(uint length)
{
    d->minQueryLength = length;
}

int AbstractRunner::minQueryLength() const
{
    return d->minQueryLength;
}

RunnerSessionData *AbstractRunner::createSessionData()
{
    return new RunnerSessionData(this);
}

void AbstractRunner::startMatch(RunnerSessionData *sessionData, const QueryContext &context)
{
    if (sessionData && sessionData->shouldStartMatch(context)) {
        match(sessionData, context);
    }
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

    exec(match);
}

bool AbstractRunner::exec(const QueryMatch &match)
{
    QString clipboardText = match.userData().toString();
    if (!clipboardText.isEmpty()) {
        QClipboard *clipboard = QApplication::clipboard();
        if (clipboard) {
            clipboard->setText(clipboardText);
            return true;
        }
    }

    return false;
}

#include "moc_abstractrunner.cpp"

