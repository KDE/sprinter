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

class AbstractRunner;

#include <runnermanager.h>

class QueryMatch
{
public:
    QueryMatch();
    QueryMatch(AbstractRunner *runner);
    QueryMatch(const QueryMatch &other);
    ~QueryMatch();

    QueryMatch &operator=(const QueryMatch &other);
    bool isValid() const;

    void setTitle(const QString &title);
    QString title() const;

    void setText(const QString &text);
    QString text() const;

    void setType(RunnerManager::MatchType type);
    RunnerManager::MatchType type() const;

    void setSource(RunnerManager::MatchSource source);
    RunnerManager::MatchSource source() const;

    /**
     * Sets if this match is a search term itself; when run
     * it will replace the current query term with its data()
     */
    void setIsSearchTerm(bool searchTerm);

    /**
     * Returns whether or not this search term represents a
     * search query term itself. Default is false
     */
    bool isSearchTerm() const;

    /**
     * User data is what ends up on e.g. the clipboard for the user to
     * later reference
     */
    void setUserData(const QVariant &data);
    QVariant userData() const;

    /**
     * Data is an internal notation for the runner to use in conjuction
     * with this match
     */
    void setData(const QVariant &data);
    QVariant data() const;

    void setPrecision(RunnerManager::MatchPrecision precision);
    RunnerManager::MatchPrecision precision() const;

    void setInternalId(const QString &id);
    QString internalId() const;

    AbstractRunner *runner() const;

private:
    class Private;
    QExplicitlySharedDataPointer<Private> d;
};

#endif
