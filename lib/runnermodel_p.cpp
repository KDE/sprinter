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

#include "runnermodel_p.h"

#include "runnermanagerthread_p.h"

#include <QDebug>
#include <QMetaEnum>

RunnerModel::RunnerModel(RunnerManagerThread *thread, QObject *parent)
    : QAbstractItemModel(parent),
      m_thread(thread),
      m_count(0)
{
    Q_ASSERT(thread);

    m_roles.insert(Qt::DisplayRole, "Name");
    m_roleColumns.append(Qt::DisplayRole);
    QMetaEnum e = metaObject()->enumerator(metaObject()->indexOfEnumerator("DisplayRoles"));
    for (int i = 0; i < e.keyCount(); ++i) {
        const int enumVal = e.value(i);
        if (enumVal == IsLoadedRole) {
            m_loadedColumn = i + 1;
        } else if (enumVal == IsBusyRole) {
            m_busyColumn = i + 1;
        }
        m_roles.insert(enumVal, e.key(i));
        m_roleColumns.append(enumVal);
    }

    connect(thread, SIGNAL(loadingRunnerMetaData()), this, SLOT(runnerMetaDataLoading()));
    connect(thread, SIGNAL(loadedRunnerMetaData()), this, SLOT(runnerMetaDataLoaded()));
    connect(thread, SIGNAL(runnerLoaded(int)), this, SLOT(runnerLoaded(int)));
    connect(thread, SIGNAL(busyChanged(int)), this, SLOT(runnerBusy(int)));
}

RunnerModel::~RunnerModel()
{
}

QVariant RunnerModel::data(const QModelIndex &index, int role) const
{
    if (!m_thread || !index.isValid() || index.parent().isValid()) {
        return QVariant();
    }

    QVector<RunnerMetaData> info = m_thread->runnerMetaData();
    if (index.row() >= info.count()) {
        return QVariant();
    }

    if (index.column() > 0 &&
        index.column() < m_roleColumns.count() &&
        role == Qt::DisplayRole) {
        role = m_roleColumns[index.column()];
    }

    switch (role) {
        case Qt::DisplayRole:
            return info[index.row()].name;
            break;
        case IdRole:
            return info[index.row()].id;
            break;
        case DescriptionRole:
            return info[index.row()].description;
            break;
        case IsLoadedRole:
            return info[index.row()].loaded;
            return true;
            break;
        case IsBusyRole:
            return info[index.row()].loaded && info[index.row()].busy;
            break;
        default:
            break;
    }

    return QVariant();
}

QVariant RunnerModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (!m_thread) {
        return QVariant();
    }

    if (section > 0 &&
        section < m_roleColumns.count() &&
        role == Qt::DisplayRole) {
        role = m_roleColumns[section];
    }

    if (orientation == Qt::Horizontal) {
        switch (role) {
            case Qt::DisplayRole:
                return tr("Name");
                break;
            case IdRole:
                return tr("ID");
                break;
            case DescriptionRole:
                return tr("Description");
                break;
            case IsLoadedRole:
                return tr("Loaded");
                break;
            case IsBusyRole:
                return tr("Busy");
                break;
            default:
                break;
        }
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}

QModelIndex RunnerModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return QModelIndex();
    }

    return createIndex(row, column);
}

QModelIndex RunnerModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

int RunnerModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_roles.count();
}

int RunnerModel::rowCount(const QModelIndex & parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_thread ? m_count : 0;
}

QHash<int, QByteArray> RunnerModel::roleNames() const
{
    return m_roles;
}

void RunnerModel::runnerMetaDataLoading()
{
    beginResetModel();
    m_count = 0;
}

void RunnerModel::runnerMetaDataLoaded()
{
    m_count = m_thread ? m_thread->runnerMetaData().count() : 0;
    emit endResetModel();
}

void RunnerModel::loadRunner(int index)
{
    if (m_thread) {
        m_thread->loadRunner(index);
    }
}

void RunnerModel::loadRunner(const QModelIndex &index)
{
    loadRunner(index.row());
}

void RunnerModel::runnerLoaded(int index)
{
    emit dataChanged(createIndex(index, m_loadedColumn), createIndex(index, m_loadedColumn));
}

void RunnerModel::runnerBusy(int index)
{
    emit dataChanged(createIndex(index, m_busyColumn), createIndex(index, m_busyColumn));
}

#include "moc_runnermodel_p.cpp"