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
      m_thread(thread)
{
    Q_ASSERT(thread);

    m_roles.insert(Qt::DisplayRole, "Title");
    m_roleColumns.append(Qt::DisplayRole);
    QMetaEnum e = metaObject()->enumerator(metaObject()->indexOfEnumerator("DisplayRoles"));
    for (int i = 0; i < e.keyCount(); ++i) {
        m_roles.insert(e.value(i), e.key(i));
        m_roleColumns.append(e.value(i));
    }
}

RunnerModel::~RunnerModel()
{
}

int RunnerModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_roles.count();
}

QVariant RunnerModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.parent().isValid()) {
        return QVariant();
    }

    if (index.column() > 0 &&
        index.column() < m_roleColumns.count() &&
        role == Qt::DisplayRole) {
        role = m_roleColumns[index.column()];
    }

    switch (role) {
        case Qt::DisplayRole:
            return QVariant();
            break;
        case IdRole:
            return QVariant();
            break;
        case DescriptionRole:
            return QVariant();
            break;
        case IsLoadedRole:
            return QVariant();
            break;
        case IsBusyRole:
            return QVariant();
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

int RunnerModel::rowCount(const QModelIndex & parent) const
{
    //TODO
    Q_UNUSED(parent)
    return m_thread ? m_thread->matchCount() : 0;
}

QHash<int, QByteArray> RunnerModel::roleNames() const
{
    return m_roles;
}

#include "moc_runnermodel_p.cpp"