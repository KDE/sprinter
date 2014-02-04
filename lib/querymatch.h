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

#ifndef QUERYMATCH
#define QUERYMATCH

#include <QExplicitlySharedDataPointer>
#include <QString>
#include <QVariant>

#include <querysession.h>

namespace Sprinter
{

class AbstractRunner;

class QueryMatch
{
public:
    QueryMatch();
    QueryMatch(AbstractRunner *runner);
    QueryMatch(const QueryMatch &other);
    ~QueryMatch();

    QueryMatch &operator=(const QueryMatch &other);
    bool operator==(const QueryMatch &rhs) const;
    bool isValid() const;

    void setTitle(const QString &title);
    QString title() const;

    void setText(const QString &text);
    QString text() const;

    void setType(QuerySession::MatchType type);
    QuerySession::MatchType type() const;

    void setSource(QuerySession::MatchSource source);
    QuerySession::MatchSource source() const;

    /**
     * Sets if this match is a search term itself; when exec'd
     * it will replace the current query term with its data().toString()
     *
     * @param searchTerm true if this is a search term, false if not
     */
    void setIsSearchTerm(bool searchTerm);

    /**
     * @return whether or not this match represents a
     * search query term itself. Default is false
     */
    bool isSearchTerm() const;

    /**
     * User data is what ends up on e.g. the clipboard for the user to
     * later reference
     *
     * @param data the user data to associate with the match
     */
    void setUserData(const QVariant &data);

    /**
     * @return the user data, if any, for this match
     */
    QVariant userData() const;

    /**
     * Data is an internal notation for the runner to use in conjuction
     * with this match
     *
     * @param data the data to associate with the match
     */
    void setData(const QVariant &data);

    /**
     * @return the data, if any, for this match
     */
    QVariant data() const;

    /**
     * Sets the precision of this match. @see QuerySession
     *
     * @param precision the precision
     */
    void setPrecision(QuerySession::MatchPrecision precision);

    /**
     * @return the precision of this match, e.g. ExactMatch
     */
    QuerySession::MatchPrecision precision() const;

    /**
     * @return a pointer to the runner that created this match. May return
     * a null pointer if the match is invalid or the runner has been deleted.
     */
    AbstractRunner *runner() const;

    /**
     * Puts the contents of userData() on the clipboard.
     * @return true on success
     */
    bool sendUserDataToClipboard() const;

private:
    class Private;
    QExplicitlySharedDataPointer<Private> d;
};

} // namespace

#endif
