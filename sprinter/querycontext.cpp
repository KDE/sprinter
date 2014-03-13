/*
 * Copyright (C) 2014 Aaron Seigo <aseigo@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "querycontext.h"
#include "querycontext_p.h"

#include <QDebug>

#include "runnersessiondata.h"
#include "runnersessiondata_p.h"

namespace Sprinter
{

void QueryContext::Private::reset(QExplicitlySharedDataPointer<Private> toDetach)
{
    // We will detach if we are a copy of someone. But we will reset
    // if we are the 'main' context others copied from. Resetting
    // one QueryContext makes all the copies obsolete.

    // We need to reset the session id of the detached instance
    // on detach to avoid receiving results for old queries
    const QUuid id = sessionId;
    sessionId = QUuid();

    toDetach.detach();

    // Now that we detached the d pointer we need to reset its q pointer
    sessionId = id;
}

QueryContext::QueryContext()
    : d(new Private)
{
}

QueryContext::QueryContext(const QueryContext &other)
{
    QReadLocker lock(&other.d->lock);
    d = other.d;
}

QueryContext::~QueryContext()
{
}

QueryContext &QueryContext::operator=(const QueryContext &other)
{
    if (this->d == other.d) {
        return *this;
    }

    QExplicitlySharedDataPointer<QueryContext::Private> oldD = d;
    d->lock.lockForWrite();
    other.d->lock.lockForRead();
    d = other.d;
    other.d->lock.unlock();
    oldD->lock.unlock();

    return *this;
}

void QueryContext::setQuery(const QString &query)
{
    QString trimmedQuery = query.trimmed();
    if (d->query == trimmedQuery) {
        return;
    }

    d->reset(d);

    d->fetchMore = false;
    d->isDefaultMatchesRequest = false;
    d->query = trimmedQuery;
}

QString QueryContext::query() const
{
    return d->query;
}

bool QueryContext::isDefaultMatchesRequest() const
{
    return d->isDefaultMatchesRequest;
}

void QueryContext::setIsDefaultMatchesRequest(bool requestDefaults)
{
    if (d->isDefaultMatchesRequest != requestDefaults) {
        d->reset(d);
        d->fetchMore = false;
        d->query.clear();
        d->isDefaultMatchesRequest = requestDefaults;
    }
}

bool QueryContext::isValid(const RunnerSessionData *sessionData) const
{
    return !d->sessionId.isNull() &&
          (!sessionData || sessionData->d->sessionId == d->sessionId);
}

bool QueryContext::networkAccessible() const
{
    return d->network->networkAccessible() == QNetworkAccessManager::Accessible;
}

void QueryContext::setFetchMore(bool fetchMore)
{
    d->fetchMore = fetchMore;
}

bool QueryContext::fetchMore() const
{
    return d->fetchMore;
}

void QueryContext::setImageSize(const QSize &size)
{
    d->imageSize = size;
}

QSize QueryContext::imageSize() const
{
    return d->imageSize;
}

void QueryContext::readLock() const
{
    d->lock.lockForRead();
}

void QueryContext::readUnlock() const
{
    d->lock.unlock();
}

} //namespace