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

#ifndef SPRINTER_RUNNERSESSIONDATA
#define SPRINTER_RUNNERSESSIONDATA

#include <sprinter/sprinter_export.h>
#include <sprinter/querymatch.h>

#include <QObject>
#include <QPointer>

namespace Sprinter
{

class Runner;
class QueryContext;
class QuerySession;

class SPRINTER_EXPORT RunnerSessionData : public QObject
{
    Q_OBJECT

public:
    /**
     * @enum MatchState Matches exist in one of two states:
     * synchronized or pending. Synchronized matches have been moved
     * from the matching threads into the main application (GUI) thread.
     * Pending matches have not yet been synchronized, but have been
     * generated and passed to the session data object as matching
     * the existing query.
     */
    enum MatchState {
        SynchronizedMatches,
        PendingMatches
    };

    /**
     * @class Busy
     * Whenever a the session data is in use, and therefore "busy",
     * an instance of this class should be instantiated. When deleted
     * the session data's "busy-ness" state will be decremented, and when
     * there are no more Busy object for the session data object it will
     * be shown as not busy. In other words: this inner class is an
     * RAII based tool for keeping track of how busy a given session data
     * object is; this is important because a session data object may
     * be busy in multiple threads or even over a spread of time due to
     * asynchronous processes.
     */
    class Busy {
        public:
            /**
             * Default constructor
             * @param sessionData the session data object that is to be
             * marked as busy; must not be a null pointer
             */
            Busy(RunnerSessionData *sessionData);
            ~Busy();
        private:
            QPointer<RunnerSessionData> m_data;
    };

    /**
     * Default constructor; created by the Runner itself when subclassed,
     * otherwise a default RunnerSessionData will be generated on behalf
     * of the Runner when required.
     * @see Runner::createSessionData
     * @param runner the runner associated with this session data object
     */
    RunnerSessionData(Runner *runner);
    virtual ~RunnerSessionData();

    /**
     * Sets the enabled state of this session data.
     * @param enabled if true, the associated runner will be included in
     * matching queries; otherwise it will be skipped. Default is true.
     */
    void setEnabled(bool enabled);

    /**
     * @return whether or not this session data object is enabled. Used
     * to decide whether to include in query matching.
     */
    bool enabled() const;

    /**
     * @return the associated runner, if any. May be null if the
     * associated runner was not set or has been deleted.
     */
    Runner *runner() const;

    /**
     * Sets the matches for a query. Used by Runners to add generated matches.
     * When called the matches are put into the Pending state for later
     * synchronization.
     * @param matches the generated matches, which will replace any current set
     * @param context the QueryContexst used in generating the matches; if the
     * context has become invalid the matches will be discarded.
     */
    void setMatches(const QVector<QueryMatch> &matches, const QueryContext &context);

    /**
     * Updates existing matches with new data. The matches are compared using
     * the content of their data() to identify which matches to update. If no
     * corresponding matches are found in either the pending or synchronized
     * states, the updated match is discarded.
     * @param matches the matches to use as updated data
     */
    void updateMatches(const QVector<QueryMatch> &matches);

    /**
     * @param state whether to return pending or synchronized matches
     * @return the current matches held by this session data object
     */
    QVector<QueryMatch> matches(MatchState state) const;

    /**
     * Sets the number of matches to return at a time, refered to as a 'page'
     * This will be set by the application code that owns a QuerySession and
     * should not be set directly.
     * @param pageSize the maximum number of matches to generate for a query
     */
    void setResultsPageSize(uint pageSize);

    /**
     * @return the maximum number of matches to generate for a query
     */
    uint resultsPageSize() const;

    /**
     * @return the offset into the result set to start at when generating matches
     * This value is automatically incremented and reset as necessary by the
     * RunnerSessionData object based on the QueryContext objects it receives.
     */
    uint resultsOffset() const;

    /**
     * @return true if the Runner is busy within this query session
     */
    bool isBusy() const;

    /**
     * Sets whether or not a Runner may return more matches for a query.
     * Used in conjunction with paging controls. This is reste to false
     * before each call of Runner::match.
     * @param hasMore true if the runner may return more matches
     * @param context the QueryContexst used in generating the matches
     */
    void setCanFetchMoreMatches(bool hasMore, const QueryContext &context);

    /**
     * @return true if the Runner claims to be able to return more matches
     * for this result set.
     */
    bool canFetchMoreMatches() const;

    /**
     * Starts a match in the Runner after performing various sanity-checks
     * on the provided QueryContext
     * @param context the QueryContext to be used in this match
     */
    void startMatch(const QueryContext &context);

Q_SIGNALS:
    /**
     * Emitted when the busy status changes
     */
    void busyChanged(bool busy);

protected:
    /**
     * This method may be overridden by subclasses
     * @return true if the Runner should start matching based on the context
     * @param context the QueryContext to use in checks
     */
    virtual bool shouldStartMatch(const QueryContext &context) const;

private:
    friend class QuerySessionThread;
    friend class QueryContext;

    class Private;
    Private * const d;
};

} // namespace

#endif