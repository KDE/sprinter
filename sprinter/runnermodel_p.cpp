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

#include "runner.h"
#include "querysessionthread_p.h"

#include <QDebug>
#include <QMetaEnum>

Q_DECLARE_METATYPE(QList<int>);

namespace Sprinter
{

RunnerModel::RunnerModel(QuerySessionThread *worker, QObject *parent)
    : QAbstractItemModel(parent),
      m_worker(worker),
      m_count(0)
{
    Q_ASSERT(worker);

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

    connect(worker, SIGNAL(loadingRunnerMetaData()), this, SLOT(runnerMetaDataLoading()));
    connect(worker, SIGNAL(loadedRunnerMetaData()), this, SLOT(runnerMetaDataLoaded()));
    connect(worker, SIGNAL(runnerLoaded(int)), this, SLOT(runnerLoaded(int)));
    connect(worker, SIGNAL(busyChanged(int)), this, SLOT(runnerBusy(int)));
    connect(worker, SIGNAL(enabledRunnersChanged()), this, SIGNAL(enabledRunnersChanged()));
}

RunnerModel::~RunnerModel()
{
}

QStringList RunnerModel::enabledRunners() const
{
    return m_worker ? m_worker->enabledRunners() : QStringList();
}

void RunnerModel::setEnabledRunners(const QStringList &runnerIds)
{
    if (m_worker) {
        QMetaObject::invokeMethod(m_worker, "setEnabledRunners", Q_ARG(QStringList, runnerIds));
    }
}

QStringList RunnerModel::runnerIds() const
{
    return m_runnerIds;
}

QVariant RunnerModel::data(const QModelIndex &index, int role) const
{
    if (!m_worker || !index.isValid() || index.parent().isValid()) {
        return QVariant();
    }

    QVector<RunnerMetaData> info = m_worker->runnerMetaData();
    if (index.row() >= info.count()) {
        return QVariant();
    }

    // QML and QWidget handle viewing models a bit differently
    // QML asks for a row and a role, basically ignoring roleColumns
    // Default QWidget views as for the DisplayRole of a row/column
    // so we adapt what data() returns based on what we are being asked for
    bool asText = false;
    if (index.column() > 0 &&
        index.column() < m_roleColumns.count() &&
        role == Qt::DisplayRole) {
        asText = true;
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
            return info[index.row()].runner != 0;
            break;
        case IsBusyRole:
            return info[index.row()].runner != 0 && info[index.row()].busy;
            break;
        case GeneratesDefaultMatchesRole:
            return info[index.row()].runner != 0 &&
                   info[index.row()].runner->generatesDefaultMatches();
            break;
        case MatchTypesRole:
            if (info[index.row()].runner) {
                if (asText) {
                    QStringList list;
                    foreach (int value, info[index.row()].runner->matchTypesGenerated()) {
                        list << textForEnum(m_worker->session(), "MatchType", value);
                    }
                    return list;
                } else {
                    QList<int> intlist;
                    foreach (int value, info[index.row()].runner->matchTypesGenerated()) {
                        intlist << value;
                    }
                    return QVariant::fromValue(intlist);
                }
            }
            break;
        case SourcesUsedRole:
            if (info[index.row()].runner) {
                if (asText) {
                    QStringList list;
                    foreach (int value, info[index.row()].runner->sourcesUsed()) {
                        list << textForEnum(m_worker->session(), "MatchSource", value);
                    }
                    return list;
                } else {
                    QList<int> intlist;
                    foreach (int value, info[index.row()].runner->sourcesUsed()) {
                        intlist << value;
                    }
                    return QVariant::fromValue(intlist);
                }
            }
            break;
        default:
            break;
    }

    return QVariant();
}

QVariant RunnerModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (!m_worker) {
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
            case GeneratesDefaultMatchesRole:
                return tr("Default Matches");
                break;
            case MatchTypesRole:
                return tr("Match Types");
                break;
            case SourcesUsedRole:
                return tr("Sources");
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

    return m_worker ? m_count : 0;
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
    m_runnerIds.clear();
    m_count = 0;

    if (m_worker) {
        QVector<RunnerMetaData> runners = m_worker->runnerMetaData();
        m_count = runners.count();

        for (int i = 0; i < m_count; ++i) {
            m_runnerIds << runners[i].id;
        }
    }

    emit runnerIdsChanged();
    endResetModel();
}

void RunnerModel::loadRunner(int index)
{
    if (m_worker) {
        QMetaObject::invokeMethod(m_worker, "loadRunner", Q_ARG(int, index));
    }
}

void RunnerModel::loadRunner(const QModelIndex &index)
{
    loadRunner(index.row());
}

void RunnerModel::runnerLoaded(int index)
{
    emit dataChanged(createIndex(index, m_loadedColumn), createIndex(index, m_roles.size()));
}

void RunnerModel::runnerBusy(int index)
{
    emit dataChanged(createIndex(index, m_busyColumn), createIndex(index, m_busyColumn));
}

} //namespace
#include "moc_runnermodel_p.cpp"