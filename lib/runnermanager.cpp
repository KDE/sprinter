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

#include "runnermanager.h"

#include <QDebug>
#include <QMetaEnum>

#include "runnermanagerthread_p.h"

class RunnerManager::Private
{
public:
    Private(RunnerManager *q)
        : thread(new RunnerManagerThread(q))
    {
    }

    RunnerManagerThread *thread;
    QString query;
    QHash<int, QByteArray> roles;
    QVector<int> roleColumns;
};

RunnerManager::RunnerManager(QObject *parent)
    : QAbstractItemModel(parent),
      d(new Private(this))
{
    d->roles.insert(Qt::DisplayRole, "Title");
    d->roleColumns.append(Qt::DisplayRole);
    QMetaEnum e = metaObject()->enumerator(metaObject()->indexOfEnumerator("DisplayRoles"));
    for (int i = 0; i < e.keyCount(); ++i) {
        d->roles.insert(e.value(i), e.key(i));
        d->roleColumns.append(e.value(i));
    }

    //TODO set row names for model
    d->thread->start();
}

RunnerManager::~RunnerManager()
{
    d->thread->exit();
    delete d;
}

void RunnerManager::setQuery(const QString &query)
{
    qDebug() << "Manager:" << QThread::currentThread() << query;
    d->query = query;
    //QMetaObject::invokeMethod(d->thread, "startQuery", Qt::AutoConnection, Q_ARG(QString, query));
    emit queryChanged(query);
}

void RunnerManager::matchesArrived()
{
    d->thread->syncMatches();
}

QString RunnerManager::query() const
{
    return d->query;
}

void RunnerManager::addingMatches(int start, int end)
{
    beginInsertRows(QModelIndex(), start, end);
}

void RunnerManager::matchesAdded()
{
    endInsertRows();
}

void RunnerManager::removingMatches(int start, int end)
{
    beginRemoveRows(QModelIndex(), start, end);
}

void RunnerManager::matchesRemoved()
{
    endRemoveRows();
}

void RunnerManager::matchesUpdated(int start, int end)
{
    emit dataChanged(createIndex(start, 0), createIndex(end, 0));
}

int RunnerManager::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return d->roles.count();
}

QVariant RunnerManager::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.parent().isValid()) {
        return QVariant();
    }

    QueryMatch match = d->thread->matchAt(index.row());

    if (index.column() > 0 &&
        index.column() < d->roleColumns.count() &&
        role == Qt::DisplayRole) {
        role = d->roleColumns[index.column()];
    }

    switch (role) {
        case Qt::DisplayRole:
            return match.title();
            break;
        case TextRole:
            return match.text();
            break;
        case TypeRole:
            return match.type();
            break;
        case PrecisionRole:
            return match.precision();
            break;
        case UserDataRole:
            return match.userData();
            break;
        case DataRole:
            return match.data();
            break;
        default:
            break;
    }

    return QVariant();
}

QVariant RunnerManager::headerData(int section, Qt::Orientation orientation, int role) const {
    if (section > 0 &&
        section < d->roleColumns.count() &&
        role == Qt::DisplayRole) {
        role = d->roleColumns[section];
    }

    if (orientation == Qt::Horizontal) {
        switch (role) {
            case Qt::DisplayRole:
                return tr("Title");
                break;
            case TextRole:
                return tr("Text");
                break;
            case TypeRole:
                return tr("Type");
                break;
            case PrecisionRole:
                return tr("Precision");
                break;
            case UserDataRole:
                return tr("User Data");
                break;
            case DataRole:
                return tr("Data");
                break;
            default:
                break;
        }
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}

QModelIndex RunnerManager::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return QModelIndex();
    }

    return createIndex(row, column);
}

QModelIndex RunnerManager::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

int RunnerManager::rowCount(const QModelIndex & parent) const
{
    //TODO
    Q_UNUSED(parent)
    return d->thread->matchCount();
}

QHash<int, QByteArray> RunnerManager::roleNames() const
{
    return d->roles;
}

#include "moc_runnermanager.cpp"
