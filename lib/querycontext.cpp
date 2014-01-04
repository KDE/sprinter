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


#include "querycontext.h"


#include <QDebug>
#include <QReadWriteLock>

class QueryContext::Private : public QSharedData
{
public:
    Private(QueryContext *context)
        : QSharedData(),
          q(context)
    {
    }


    Private(const Private &p)
        : QSharedData(),
          q(p.q)
    {
        //kDebug() << "¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿boo yeah" <<
        //type;
    }

    QString query;
    QReadWriteLock lock;
    QueryContext *q;
};

QueryContext::QueryContext()
    : d(new Private(this))
{
}

QueryContext::QueryContext(const QueryContext &other)
{
    other.d->lock.lockForRead();
    d = other.d;
    other.d->lock.unlock();
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

void QueryContext::reset()
{
    // We will detach if we are a copy of someone. But we will reset
    // if we are the 'main' context others copied from. Resetting
    // one QueryContext makes all the copies obsolete.

    // We need to mark the q pointer of the detached QueryContextPrivate
    // as dirty on detach to avoid receiving results for old queries
    d->q = 0;

    d.detach();
    d->query.clear();

    // Now that we detached the d pointer we need to reset its q pointer
    d->q = this;
}

void QueryContext::setQuery(const QString &query)
{
    d->query = query;
}

QString QueryContext::query() const
{
    return d->query;
}

bool QueryContext::isValid() const
{
    return d->q;
}
