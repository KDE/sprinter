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

#ifndef SPRINTER_QUERYMATCH
#define SPRINTER_QUERYMATCH

#include <sprinter/querysession.h>
#include <sprinter/sprinter_export.h>

#include <QExplicitlySharedDataPointer>
#include <QImage>
#include <QString>
#include <QVariant>

namespace Sprinter
{

class Runner;
class RunnerSessionData;

class SPRINTER_EXPORT QueryMatch
{
public:
    /**
     * Default constructor, which creates an invalid match. A match is
     * valid once it has been passed to a RunnerSessionData object.
     */
    QueryMatch();

    /**
     * Copy constructor; valid if the other match is also valid. QueryMatch is
     * implicitly shared so this is fast
     */
    QueryMatch(const QueryMatch &other);

    ~QueryMatch();

    QueryMatch &operator=(const QueryMatch &other);
    bool operator==(const QueryMatch &rhs) const;

    /**
     * @returns true if this match is valid and can be used.
     */
    bool isValid() const;

    /**
     * Sets a title for this match; should be translated if possible.
     * Every query should have a title, and it should be short and meaningful.
     * @param title the test to use as the title
     */
    void setTitle(const QString &title);

    /**
     * @return the title for this match
     */
    QString title() const;

    /**
     * Sets some optional descriptive text for this match. This text allows
     * the UI to show the user more context or details to support the title.
     * This text may be of arbitrary length, but keeping it to a simple phrase
     * or two is recommended.
     * It should be translated if possible.
     * @param text the text to use
     */
    void setText(const QString &text);

    /**
     * @return the descriptive text for this match
     */
    QString text() const;

    /**
     * Sets the type of the match; this should always be set.
     * @param type type of the match
     */
    void setType(QuerySession::MatchType type);

    /**
     * @return the type of this match
     */
    QuerySession::MatchType type() const;

    /**
     * Sets the source that this match came from. Useful for sortings
     * and filtering in user interfaces. Should always be set.
     * @param source where the match came from
     */
    void setSource(QuerySession::MatchSource source);

    /**
     * @return where this match came from
     */
    QuerySession::MatchSource source() const;

    /**
     * Sets an image to be displayed along with this result
     * It should be sized within the bounds of QueryContext::imageSize
     *
     * @param image the image to use with this match
     */
    void setImage(const QImage &image);

    /**
     * @return the image associated with this match (if any)
     */
    QImage image() const;

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
     * @return a pointer to the sessionData associated with this match
     * May return a null pointer if the match is invalid.
     */
    RunnerSessionData *sessionData() const;

    /**
     * @return a pointer to the runner that created this match. May return
     * a null pointer if the match is invalid.
     */
    Runner *runner() const;

    /**
     * Puts the contents of userData() on the clipboard.
     * @return true on success
     */
    bool sendUserDataToClipboard() const;

private:
    friend class RunnerSessionData;

    class Private;
    QExplicitlySharedDataPointer<Private> d;
};

} // namespace

#endif
