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

#include <sprinter/querymatch.h>
#include <sprinter/sprinter_export.h>

#include <QExplicitlySharedDataPointer>
#include <QString>
#include <QSize>

namespace Sprinter
{

/**
 * @class QueryContext
 * This class represents the a query and all relevant context to it.
 * It is passed to Runners and RunnerSessionData objects both to notify
 * of the current query and as part of thread synchronization processes
 * via the isValid() state.
 *
 * With copy-on-write style semantics, when the query state is changed
 * on one object it detaches from all copies, causing all other copies
 * to become invalid.
 *
 * Only the QuerySession object should set the query state. To all other
 * users it is a read-only object.
 */
class SPRINTER_EXPORT QueryContext
{
public:
    /**
     * Default constructor which creates an empty QueryContext
     * */
    QueryContext();

    /**
     * Copy constructor
     */
    QueryContext(const QueryContext &other);

    /**
     * Default destructor. Non-virtual as this class is not meant
     * to be subclassed.
     */
    ~QueryContext();

    /**
     * Assignment operator
     */
    QueryContext &operator=(const QueryContext &other);

    /**
     * Sets the current query string. Will invalidate copies of this object
     * which may still exist. Setting the query string will also reset
     * the default match request and fetching more states to false.
     * @see isDefaultMatchesRequest
     * @see fetchMore
     * @param query the query string
     */
    void setQuery(const QString &query);

    /**
     * @return the current query string
     */
    QString query() const;

    /**
     * @return true if this is a request for default matches.
     * The query string may be empty at this point.
     */
    bool isDefaultMatchesRequest() const;

    /**
     * Sets if this is a request for default matches.
     * @see Runner::generatesDefaultMatches
     * @param requestDefault true if this is a default match request
     */
    void setIsDefaultMatchesRequest(bool requestDefaults);

    /**
     * @return true if this object is still valid, e.g. it represents an
     * ongoing query.
     */
    bool isValid() const;

    /**
     * @return true if the newtork is accessible
     */
    bool networkAccessible() const;

    /**
     * Sets whether this is a request to fetch another page of matches
     * @param fetchMore true if this is a request to fetch more matches
     */
    void setFetchMore(bool fetchMore);

    /**
     * @return true if this is a request to fetch more matches. Used by
     * RunnerSessionData to set the match paging values correctly.
     */
    bool fetchMore() const;

    /**
     * Sets the size of images that Runner should generate for matches
     * @param size the image size desired
     */
    void setImageSize(const QSize &size);

    /**
     * @return the size of images that Runner should generate for matches
     */
    QSize imageSize() const;

    /**
     * A higher-order function which runs the passed in lambda function
     * if and only if the context is valid. This prevents race conditions
     * that would otherwise exist in test-then-run patterns across multiple
     * threads.
     * @param algorithm the lambda function to execute
     */
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
