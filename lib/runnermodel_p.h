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

class RunnerManagerThread;

class RunnerModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum DisplayRoles {
        IdRole = Qt::UserRole,
        DescriptionRole,
        IsLoadedRole,
        IsBusyRole
    };
    Q_ENUMS(DisplayRoles)

    RunnerModel(RunnerManagerThread *thread, QObject *parent = 0);
    ~RunnerModel();

    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QHash<int, QByteArray> roleNames() const;

public Q_SLOTS:
    void loadRunner(int index);
    void loadRunner(const QModelIndex &index);

private Q_SLOTS:
    void runnerMetaDataLoading();
    void runnerMetaDataLoaded();
    void runnerLoaded(int);

//TODO: add slots to load/unload runners by index (int/QModelIndex)
private:
    QPointer<RunnerManagerThread> m_thread;
    QHash<int, QByteArray> m_roles;
    QVector<int> m_roleColumns;
    int m_count;
};

#endif

