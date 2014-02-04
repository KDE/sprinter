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

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QPointer>

namespace Sprinter
{

class QueryMatch::Private : public QSharedData
{
public:
    Private(AbstractRunner *r)
        : runner(r),
          type(QuerySession::UnknownType),
          source(QuerySession::FromInternalSource),
          precision(QuerySession::UnrelatedMatch),
          isSearchTerm(false)
    {
    }

    QPointer<AbstractRunner> runner;
    QString title;
    QString text;
    QuerySession::MatchType type;
    QuerySession::MatchSource source;
    QuerySession::MatchPrecision precision;
    QVariant data;
    QVariant userData;
    bool isSearchTerm;
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

bool QueryMatch::operator==(const QueryMatch &rhs) const
{
    return d == rhs.d;
}

bool QueryMatch::isValid() const
{
    return d->runner;
}

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

void QueryMatch::setType(QuerySession::MatchType type)
{
    d->type = type;
}

void QueryMatch::setSource(QuerySession::MatchSource source)
{
    d->source = source;
}

QuerySession::MatchSource QueryMatch::source() const
{
    return d->source;
}

void QueryMatch::setUserData(const QVariant &data)
{
    d->userData = data;
}

QVariant QueryMatch::userData() const
{
    return d->userData;
}

void QueryMatch::setData(const QVariant &data)
{
    d->data = data;
}

QVariant QueryMatch::data() const
{
    return d->data;
}

QuerySession::MatchType QueryMatch::type() const
{
    return d->type;
}

void QueryMatch::setPrecision(QuerySession::MatchPrecision precision)
{
    d->precision = precision;
}

QuerySession::MatchPrecision QueryMatch::precision() const
{
    return d->precision;
}

AbstractRunner *QueryMatch::runner() const
{
    return d->runner;
}

void QueryMatch::setIsSearchTerm(bool searchTerm)
{
    d->isSearchTerm = searchTerm;
}

bool QueryMatch::isSearchTerm() const
{
    return d->isSearchTerm;
}

bool QueryMatch::sendUserDataToClipboard() const
{
    QString clipboardText = userData().toString();
    if (!clipboardText.isEmpty()) {
        QClipboard *clipboard = QApplication::clipboard();
        if (clipboard) {
            clipboard->setText(clipboardText);
            return true;
        }
    }

    return false;
}

} // namespace