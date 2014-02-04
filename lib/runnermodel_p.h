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

#ifndef RUNNERMODEL
#define RUNNERMODEL

#include <QAbstractItemModel>
#include <QPointer>
#include <QStringList>

class QuerySessionThread;

class RunnerModel : public QAbstractItemModel
{
    Q_OBJECT

    Q_PROPERTY(QStringList enabledRunners WRITE setEnabledRunners
                                          READ enabledRunners
                                          NOTIFY enabledRunnersChanged)
    Q_PROPERTY(QStringList runnerIds READ runnerIds
                                     NOTIFY runnerIdsChanged)

public:
    enum DisplayRoles {
        IdRole = Qt::UserRole,
        DescriptionRole,
        IsLoadedRole,
        IsBusyRole,
        GeneratesDefaultMatchesRole
    };
    Q_ENUMS(DisplayRoles)

    RunnerModel(QuerySessionThread *thread, QObject *parent = 0);
    ~RunnerModel();

    QStringList enabledRunners() const;
    void setEnabledRunners(const QStringList &runnerIds);

    QStringList runnerIds() const;

public Q_SLOTS:
    void loadRunner(int index);
    void loadRunner(const QModelIndex &index);

Q_SIGNALS:
    void enabledRunnersChanged();
    void runnerIdsChanged();

private Q_SLOTS:
    void runnerMetaDataLoading();
    void runnerMetaDataLoaded();
    void runnerLoaded(int);
    void runnerBusy(int);

private:
    QPointer<QuerySessionThread> m_thread;
    QHash<int, QByteArray> m_roles;
    QVector<int> m_roleColumns;
    QStringList m_runnerIds;
    int m_count;
    int m_loadedColumn;
    int m_busyColumn;

public:
    // All the QAbstractItemModel reimp's
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QHash<int, QByteArray> roleNames() const;
};

#endif

