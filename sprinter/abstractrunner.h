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

#ifndef SPRINTER_RUNNER
#define SPRINTER_RUNNER

#include <sprinter/sprinter_export.h>

#include <QObject>
#include <QRunnable>

#include <sprinter/querycontext.h>
#include <sprinter/runnersessiondata.h>

namespace Sprinter
{

class QueryMatch;
class QueryContext;
class RunnerSessionData;

class SPRINTER_EXPORT Runner : public QObject
{
    Q_OBJECT

public:
    Runner(QObject *parent);
    ~Runner();

    /**
     * The unique internal ID for this runner. This is retrieved
     * from the plugin's metadata.
     */
    QString id() const;

    /**
     * @return a new RunnerSessionData object for use in this session
     *
     * Runners which need to set up, access and/or store data members
     * or other resources that should span an entire query session
     * may reimplement this method to return a RunnerSessionData
     * subclass.
     *
     * If not reimplemented, a generic RunnerSessionData object will
     * be created on behalf of the runner.
     *
     * A new object must be returned every time this method is called
     * and ownership of the resulting RunnerSessionData object is
     * given to the caller. Runners must not manage the lifecycle of
     * this object, e.g. by caching it internally.
     */
    virtual RunnerSessionData *createSessionData();

    /**
     * Called when a match is to be made. Generally this should not be
     * called directly, but instead RunnerSessionData::startMatch should
     * be called which will check if the match should be run and do additional
     * session related bookkeeping.
     * 
     * @param sessionData this is the RunnerSessionData created by
     * the runner (or on behalf of it) when the match session was
     * initiated. Guaranteed to never be null; must not be deleted.
     * @param context this object contains the query text itself and
     * additional metadata. @see QueryContext
     */
    virtual void match(RunnerSessionData *sessionData, const QueryContext &context);

    /**
     * When a match is to be exec'd, this method can be called.
     * A number of pre-exec checks are done and if they pass then
     * the runner's implementation of @see exec is called
     */
    bool startExec(const QueryMatch &match);

    /**
     * @return the  number of characters a query term must have
     * minimally to be of interest to the runner. Defaults to 3.
     */
    uint minQueryLength() const;

    /*
     * @return the types of results this runner may return
     *
     * If the list is empty, then the type must be considered as undeterminded
     * and the runner should always be included in query matching. Otherwise,
     * this runner may be excluded from matching based on this list.
     */
    QVector<QuerySession::MatchType> matchTypesGenerated() const;

    /*
     * @return the types of sources this runner uses to create mataches
     *
     * If the list is empty, then the source must be considered as undeterminded
     * and the runner should always be included in query matching. Otherwise,
     * this runner may be excluded from matching based on this list.
     */
    QVector<QuerySession::MatchSource> sourcesUsed() const;

    /**
     * @return true if this runner has a default set of results that
     * can be returned without any search term. False by default.
     */
    bool generatesDefaultMatches() const;

protected:
    /**
     * Sets the minimum query length accepted by this runner
     * Used by pre-match checks in @see startMatch, for instance.
     *
     * @param legnth the number of characters a query term must have
     * minimally to be of interest to the runner
     */
    void setMinQueryLength(uint length);

    /**
     * Called when a match is to be executed. What that means
     * precisely is up to the runner. The match is guaranteed
     * to have been issued by this runner and be valid.
     *
     * If a runner does not reimplement this method, then if the
     * userData() is valid, it will be placed on the clipboard when
     * exec'd. A runner may also place a match's contents on
     * the clipboard by calling @see QueryMatch::sendUserDataToClipboard()
     *
     * Note: QueryMatch objects which are marked as search terms
     * will not be passed to this method
     */
    virtual bool exec(const QueryMatch &match);

    /**
     * Used to set whether or not this runner has a default set of results that
     * can be returned without any search term.
     *
     * Must be called in the runner's constructor to take effect.
     * @param hasDefaultsMatches true if there are default matches
     */
    void setGeneratesDefaultMatches(bool hasDefaultMatches);

    /**
     * Sets the types of matches this runner may generate in response to @see match
     * being called. The runner may not always generate matches of all types in the
     * list, but any matches generated should be of a type in the list set.
     *
     * An empty list simply means "unknown" and the runner may generate any sort of
     * match. This is the default.
     *
     * It is highly recommended to set this to an accurate list of match types in the
     * constructor of the runner.
     */
    void setMatchTypesGenerated(const QVector<QuerySession::MatchType> types);

    /**
     * Sets the sources which this runner uses to generate its matches
     * This allows things such as runners requiring network to be filtered out
     * if there is no network available.
     * Must be called in the runner's constructor to take effect.
     */
    void setSourcesUsed(const QVector<QuerySession::MatchSource> &sources);

private:
    friend class QuerySessionThread;

    class Private;
    Private * const d;
};

} //namespace

#endif