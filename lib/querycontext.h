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

#ifndef QUERYCONTEXT
#define QUERYCONTEXT

#include <querymatch.h>

#include <QExplicitlySharedDataPointer>
#include <QString>

namespace Sprinter
{

class QueryContext
{
public:
    QueryContext();
    QueryContext(const QueryContext &other);
    ~QueryContext();

    QueryContext &operator=(const QueryContext &other);

    void setQuery(const QString &query);
    QString query() const;

    bool isDefaultMatchesRequest() const;
    void setIsDefaultMatchesRequest(bool requestDefaults);

    bool isValid() const;

    bool networkAccessible() const;

    void setFetchMore(bool fetchMore);
    bool fetchMore() const;

    template<typename Func>
    bool ifValid(Func algorithm) const {
        readLock();
        bool rv = false;

        if (isValid()) {
            rv = algorithm();
        }

        readUnlock();
        return rv;
    }

private:
    void readLock() const;
    void readUnlock() const;

    class Private;
    QExplicitlySharedDataPointer<Private> d;
};

} // namespace
#endif
