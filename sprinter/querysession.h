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

#include <sprinter/sprinter_export.h>

#include <QAbstractItemModel>

namespace Sprinter
{

class QueryMatch;

class SPRINTER_EXPORT QuerySession : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(QString query WRITE setQuery READ query NOTIFY queryChanged)
    Q_PROPERTY(QAbstractItemModel *runnerModel READ runnerModel CONSTANT)
    Q_PROPERTY(QSize imageSize WRITE setImageSize READ imageSize NOTIFY imageSizeChanged)

public:
    enum DisplayRoles {
        TextRole = Qt::UserRole,
        ImageRole,
        TypeRole,
        SourceRole,
        PrecisionRole,
        UserDataRole,
        DataRole,
        SearchTermRole,
        RunnerRole,
        ExecutingRole
    };
    Q_ENUMS(DisplayRoles)

    enum MatchType {
        UnknownType = 0,

        // Generic file types
        ExecutableType, // an executable file (application, command line, e.g.)
        FileType, // only use if there is not a relevant Content type
        InstallableType, // Something that is installable, e.g. an app or package
        PathType, // a path in the local filesystem

        // Resource address types
        FilesystemLocationType, // a non-literal location in the local fs
        HardwareType, // a hardware device
        NetworkLocationType, // a URI pointing to a resource on the network

        // Information types
        DateTimeType, // dates and time
        GeolocationType, // geographical
        LanguageType, // translations, definitions, etc.
        MathAndUnitsType, // numerical and calculations
        EnvironmentalType, // weather, etc.

        // Content types
        AlbumType, // full audio album
        AudioType, // audio data (music, etc.)
        BookmarkType, // web or local bookmark
        BookType, // eBooks
        ContactType, // person, e.g.
        DocumentType, // word processing, spreadsheet, etc.
        EventType, // calendaring
        MagazineType, // magazines
        MessageType, // email, IM, SMS, etc.
        VideoType, // video

        // Desktop shell and user interface types
        ActivityType, // a desktop activity
        AppActionType, // an action in an external app; perhaps from a menu
        AppSessionType, // a desktop app session within the current user session
        DesktopType, // virtual desktop name/numbers
        UserSessionType, // a user session
        WindowType, // a window on the desktop
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

    QuerySession(QObject *parent = 0);
    ~QuerySession();

    /**
     * A model containing a list of runners, their status and controls.
     *
     * The model provides the following roles:
     *   DisplayRole: the user visible name of the runner
     *   IdRole: the internal id of the runner
     *   DescriptionRole: a user visible description
     *   IsLoadedRole: true if the runner plugin is loaded
     *   IsBusyRole: true if the runner is busy processing a query
     *   GeneratesDefaultMatchesRole: true if the runner can generate a set of
     *                                default matches
     *   MatchTypesRole: the types of matches the runner can produce
     *                   @see QuerySession::MatchType
     *   SourcesUsedRole: the sources used by the runner for generating matches
     *                   @see QuerySession::MatchSource
     *
     * Additionally, the model provides control over the runners with
     * the following properties and methods which are available via the
     * QProperty sytem to both C++ and QML users:
     *   QStringList runnerIds: all runner ids; redundant with the IdRole but
     *                          useful in QML to do things like:
     *       session.runnerModel.enabledRunners = session.runnerModel.runnerIds
     *   QStringList enabledRunners: the list of currently enabled runners
     *   loadRunner: taking either an integer or a QModelIndex, this will cause
     *               the plugin at that index to be loaded
     **/
    QAbstractItemModel *runnerModel() const;

    /**
     * Returns the current query string
     * @see setQuery
     */
    QString query() const;

    /**
     * Sets the size of image that matches should generate for their
     * icon or previews
     * @param size the desired size of images
     */
    void setImageSize(const QSize &size);

    /**
     * @return the size of images matches should generate
     */
    QSize imageSize() const;

public Q_SLOTS:
    /**
     * Executes a request for the default match set from all enabled runners
     */
    void requestDefaultMatches();

    /**
     * Executes a request for more matches on the current query
     */
    void requestMoreMatches();

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
     * @see halt
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
    void halt();

Q_SIGNALS:
    /**
     * This is emitted whenever the query string changes
     * @see setQuery
     */
    void queryChanged(const QString &query);

    /**
     * Emitted when the size of match images changes
     */
    void imageSizeChanged(const QSize &size);

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

    Q_PRIVATE_SLOT(d, void startMatchSynchronization());
    Q_PRIVATE_SLOT(d, void resetModel());
    Q_PRIVATE_SLOT(d, void executionFinished(const QueryMatch &match, bool success));
};

} // namespace

#endif