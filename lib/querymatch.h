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

class QueryMatch
{
public:
    //TODO organize these in a more orderl fashion
    enum Type {
        UnknownType = 0,
        ExecutableType,
        FileType,
        MathAndUnitsType,
        DocumentType,
        BookType,
        AlbumType,
        AudioType,
        VideoType,
        FilesystemLocationType,
        NetworkLocationType,
        ContactType,
        EventType,
        MessageType,
        BookmarkType,
        DesktopType,
        WindowType,
        HardwareType,
        AppActionType,
        AppSessionType,
        LocationType,
        LanguageType,
        DateTimeType,
        InstallableType
    };

    enum Source {
        FromInternalSource = 0,
        FromFilesystem,
        FromLocalIndex, // file indexing, db, etc
        FromLocalService, // o.s., middelware, user session, etc.
        FromDesktopShell,
        FromNetworkService
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
    bool isValid() const;

    void setTitle(const QString &title);
    QString title() const;

    void setText(const QString &text);
    QString text() const;

    void setType(Type type);
    Type type() const;

    void setSource(Source source);
    Source source() const;

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

    void setPrecision(Precision precision);
    Precision precision() const;

    void setInternalId(const QString &id);
    QString internalId() const;

    //TODO run match, icon, actions (sub-QueryMatches?), id (QUuid?)
private:
    class Private;
    QExplicitlySharedDataPointer<Private> d;
};

#endif
