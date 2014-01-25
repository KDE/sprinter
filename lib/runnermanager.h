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

#ifndef RUNNERMANAGER
#define RUNNERMANAGER

#include <QAbstractItemModel>

class QueryMatch;

class RunnerManager : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(QString query WRITE setQuery READ query NOTIFY queryChanged)
    Q_PROPERTY(QAbstractItemModel *runnerModel READ runnerModel CONSTANT)

public:
    enum DisplayRoles {
        TextRole = Qt::UserRole,
        TypeRole,
        SourceRole,
        PrecisionRole,
        UserDataRole,
        DataRole,
        SearchTermRole,
        RunnerRole
    };
    Q_ENUMS(DisplayRoles)

    //TODO organize these in a more orderly fashion
    enum MatchType {
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
        GeolocationType,
        LanguageType,
        DateTimeType,
        InstallableType
    };
    Q_ENUMS(MatchType)

    enum MatchSource {
        FromInternalSource = 0,
        FromFilesystem,
        FromLocalIndex, // file indexing, db, etc
        FromLocalService, // o.s., middelware, user session, etc.
        FromDesktopShell,
        FromNetworkService
    };
    Q_ENUMS(MatchSource)

    enum MatchPrecision {
        UnrelatedMatch = 0,
        FuzzyMatch,
        CloseMatch,
        ExactMatch
    };
    Q_ENUMS(MatchPrecision)

    RunnerManager(QObject *parent = 0);
    ~RunnerManager();

    /**
     * A model containing a list of runners and their status
     * Also provides access to loading and unloading them
     **/
    QAbstractItemModel *runnerModel() const;

    /**
     * Returns the current query string
     * @see setQuery
     */
    QString query() const;

public Q_SLOTS:
    /**
     * Sets the text to be used as the query string
     * This may be called multiple times while querying, allowing
     * for "search as you type" patterns.
     *
     * When called, this will create a query session internally:
     * each active runner will be asked to create a RunnerSessionData
     * object and they may initialize various data members and
     * communication channels as needed to complete queries. To
     * release these resources at the end of a query session
     * (e.g. after the user dimisses the search UI) call
     * @see endQuerySession
     */
    void setQuery(const QString &query);

    /**
     * Executes the match at the given index
     * @see executionStarted @see executionFinished
     * @param index the row number of the match to execute
     */
    void executeMatch(int index);

    /**
     * Executes the match at the given index
     * @see executionStarted @see executionFinished
     * @param index the row number of the match to execute
     */
    void executeMatch(const QModelIndex &index);

    /**
     * This method should be called when a query session is complete.
     * It allows the manager to clean up unused resources created by
     * runners when a search session is finished.
     *
     * A session is begun automatically when setQuery is called
     * and no session has been started, so there is no startSession
     * method.
     *
     * Calling this method before the query session is fully complete
     * will result in any current matches being removed. Otherwise
     * there are no negative side effects to calling this at the "wrong"
     * time other than incuring additional overhead when setQuery is
     * next called.
     */
    void endQuerySession();

Q_SIGNALS:
    /**
     * When a match is executed, this signal is emitted. This gives
     * applications the opportunity to provide user feedback when
     * a match begins
     */
    void executionStarted(const QueryMatch &match);

    /**
     * When a match has been executed, this signal is emitted. This
     * gives applications the opportunity to provide user feedback when
     * execution is complete as well as a way to know when to call
     * @see endQuerySession
     */
    void executionFinished(const QueryMatch &match, bool success);

    /**
     * This is emitted whenever the query string changes
     * @see setQuery
     */
    void queryChanged(const QString &query);

public:
    // The reimplemented model API follows below:
    /**
     * @reimp QAbstractItemModel
     */
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    /**
     * @reimp QAbstractItemModel
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    /**
     * @reimp QAbstractItemModel
     */
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

    /**
     * @reimp QAbstractItemModel
     */
    QModelIndex parent(const QModelIndex &index) const;

    /**
     * @reimp QAbstractItemModel
     */
    int rowCount(const QModelIndex & parent = QModelIndex()) const;

    /**
     * @reimp QAbstractItemModel
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    /**
     * @reimp QAbstractItemModel
     */
    QHash<int, QByteArray> roleNames() const;

private:
    // these methods are for RunnserSessionData class (e.g. in syncMatches) only
    friend class RunnerSessionData;

    class Private;
    friend class Private;
    Private * const d;

    Q_PRIVATE_SLOT(d, void resetModel());
};

#endif

