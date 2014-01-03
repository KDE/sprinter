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

class RunnerManager : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(QString query WRITE setQuery READ query NOTIFY queryChanged)

public:
    enum Roles {
        TextRole = Qt::UserRole,
        TypeRole,
        PrecisionRole,
        UserDataRole,
        DataRole
    };
    RunnerManager(QObject *parent = 0);
    ~RunnerManager();

    QString query() const;

    void matchesArrived();

    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex & parent = QModelIndex()) const;

    // setRunners
    // listRunners

public Q_SLOTS:
    void setQuery(const QString &query);

Q_SIGNALS:
    void queryChanged(const QString &query);
    void querySessionCompleted();

private:
    // these methods are for RunnserSessionData class (e.g. in syncMatches) only
    friend class RunnerSessionData;
    void addingMatches(int start, int end);
    void matchesAdded();
    void removingMatches(int start, int end);
    void matchesRemoved();
    void matchesUpdated(int start, int end);

    class Private;
    Private * const d;
};

#endif

