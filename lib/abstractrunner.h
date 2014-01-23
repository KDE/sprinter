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

#ifndef ABSTRACTRUNNER
#define ABSTRACTRUNNER

#include <QObject>
#include <QRunnable>

#include <querycontext.h>
#include <runnersessiondata.h>

class QueryMatch;
class AbstractRunner;
class QueryContext;
class RunnerSessionData;

class AbstractRunner : public QObject
{
    Q_OBJECT

public:
    AbstractRunner(QObject *parent);
    ~AbstractRunner();

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
     * This method will start a match. A number of pre-match
     * checks are done and if they pass then the runner's implementation
     * of @see match is called.
     *
     * @param sessionData the RunnerSessionData object from the query
     * session for this runner. This object must have come from an
     * earlier call to @see createSessionData.
     * @param context the current search query data
     */
    void startMatch(RunnerSessionData *sessionData, const QueryContext &context);

    /**
     * When a match is to be exec'd, this method can be called.
     * A number of pre-exec checks are done and if they pass then
     * the runner's implementation of @see exec is called
     */
    bool startExec(const QueryMatch &match);

    /**
     * Sets the minimum query length accepted by this runner
     * Used by pre-match checks in @see startMatch, for instance.
     *
     * @param legnth the number of characters a query term must have
     * minimally to be of interest to the runner
     */
    void setMinQueryLength(uint length);

    /**
     * @return the  number of characters a query term must have
     * minimally to be of interest to the runner. Defaults to 3.
     */
    int minQueryLength() const;

protected:
    /**
     * Called when a match is to be made.
     * @param sessionData this is the RunnerSessionData created by
     * the runner (or on behalf of it) when the match session was
     * initiated. Guaranteed to never be null; must not be deleted.
     * @param context this object contains the query text itself and
     * additional metadata. @see QueryContext
     */
    virtual void match(RunnerSessionData *sessionData, const QueryContext &context);

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

private:
    friend class RunnerFactory;
    /**
     * Allows setting the internal ID for the runner. This should
     * not be called by the runner itself, but rather will be set
     * for it on plugin load.
     */
    void setId(const QString &newId);

    class Private;
    Private * const d;
};

class AbstractRunnerFactory : public QObject
{
    Q_OBJECT

public:
    AbstractRunnerFactory(QObject *parent = 0) : QObject(parent) {}
    virtual AbstractRunner *create(const QString &id, QObject *parent = 0)
    {
        return 0;
    }
};

#define RUNNER_FACTORY(type, id, json) \
class RunnerFactory : public AbstractRunnerFactory { \
    Q_OBJECT \
    Q_PLUGIN_METADATA(IID #id FILE #json) \
public: \
    RunnerFactory(QObject *parent = 0) : AbstractRunnerFactory(parent) {} \
    AbstractRunner *create(const QString &runnerId, QObject *parent = 0) {\
        type *r = new type(parent);\
        r->setId(runnerId);\
        return r;\
    }\
};

#endif