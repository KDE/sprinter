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

#include <QString>
#include <QExplicitlySharedDataPointer>

class AbstractRunner;

class QueryMatch
{
public:
    enum Type {
        UnknownType = 0,
        InformationalType,
        FileType,
        ExecutableType,
        NetworkLocationType,
        ContactType,
        EventType,
        BookmarkType,
        DesktopShellType,
        HardwareType,
        AppActionType,
        AppSessionType,
        LocationType,
        LanguageType
    };

    enum Precision {
        UnrelatedMatch = 0,
        FuzzyMatch,
        CloseMatch,
        ExactMatch
    };

    QueryMatch();
    QueryMatch(AbstractRunner *runner);
    QueryMatch(const QueryMatch &other);
    ~QueryMatch();

    QueryMatch &operator=(const QueryMatch &other);

    void setTitle(const QString &title);
    QString title() const;

    void setText(const QString &text);
    QString text() const;

    void setType(Type type);
    Type type() const;

    void setPrecision(Precision precision);
    Precision precision() const;

    /**
     * @arg interval the number of seconds with which to attempt to update
     * this match; 0 means "never" and is the default
     */
    void setUpdateInterval(uint interval);
    uint updateInterval() const;

    void setInternalId(const QString &id);
    QString internalId() const;

    //TODO run match, icon, actions (sub-QueryMatches?), id (QUuid?)
private:
    class Private;
    QExplicitlySharedDataPointer<Private> d;
};

#endif
