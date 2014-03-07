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

#ifndef QUERYCONTEXT_PRIVATE
#define QUERYCONTEXT_PRIVATE

#include <QReadWriteLock>
#include <QNetworkAccessManager>
#include <QUuid>

namespace Sprinter
{

class QueryContext::Private : public QSharedData
{
public:
    Private(QueryContext *context)
        : QSharedData(),
          q(context),
          network(new QNetworkAccessManager),
          imageSize(64, 64),
          fetchMore(false),
          isDefaultMatchesRequest(false)
    {
    }


    Private(const Private &p)
        : QSharedData(),
          q(p.q)
    {
        //kDebug() << "¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿boo yeah" <<
        //type;
    }

    void reset(QExplicitlySharedDataPointer<Private> toDetach, QueryContext *newQ);

    QueryContext *q;
    QString query;
    QReadWriteLock lock;
    QSharedPointer<QNetworkAccessManager> network;
    QSize imageSize;
    QUuid sessionId;
    bool fetchMore;
    bool isDefaultMatchesRequest;
};

} // namespace

#endif