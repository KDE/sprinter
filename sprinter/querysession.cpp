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

#include "querysession.h"
#include "querysession_p.h"

#include <QDebug>
#include <QMetaEnum>
#include <QThreadPool>

#include "runner.h"
#include "querysessionthread_p.h"
#include "runnermodel_p.h"

namespace Sprinter
{

QuerySession::Private::Private(QuerySession *session)
    : q(session),
      workerThread(new QThread(q)),
      worker(new QuerySessionThread(q)),
      runnerModel(new RunnerModel(worker, q)),
      syncTimer(new NonRestartingTimer(q)),
      matchesArrivedWhileExecuting(false)
{
    qRegisterMetaType<QUuid>("QUuid");
    qRegisterMetaType<Sprinter::QueryContext>("Sprinter::QueryContext");
    qRegisterMetaType<Sprinter::QueryMatch>("Sprinter::QueryMatch");

    q->connect(worker, SIGNAL(resetModel()), q, SLOT(resetModel()));

    roles.insert(Qt::DisplayRole, "Title");
    roleColumns.append(Qt::DisplayRole);
    QMetaEnum e = q->metaObject()->enumerator(q->metaObject()->indexOfEnumerator("DisplayRoles"));
    for (int i = 0; i < e.keyCount(); ++i) {
        const int enumVal = e.value(i);
        if (enumVal == ImageRole) {
            imageRoleColumn = i + 1;
        }
        roles.insert(enumVal, e.key(i));
        roleColumns.append(enumVal);
    }

    // if synchronization becomes too slow, it could be moved to happen
    // in this worker thread, but only with significant complexity
    syncTimer->setInterval(10);
    syncTimer->setSingleShot(true);

    connect(syncTimer, SIGNAL(timeout()),
            q, SLOT(startMatchSynchronization()));

    // when the thread exits, the worker object should
    // be deleted. the thread is exited in ~QuerySession
    connect(workerThread, SIGNAL(finished()),
            worker, SLOT(deleteLater()));

    qDebug() << "Starting QuerySession worker thread from"
             << QThread::currentThread();
    workerThread->start();
    worker->moveToThread(workerThread);
    QMetaObject::invokeMethod(worker, "loadRunnerMetaData");
}

void QuerySession::Private::resetModel()
{
    q->beginResetModel();
    q->endResetModel();
}

void QuerySession::Private::addingMatches(int start, int end)
{
    q->beginInsertRows(QModelIndex(), start, end);
}

void QuerySession::Private::matchesAdded()
{
    q->endInsertRows();
}

void QuerySession::Private::removingMatches(int start, int end)
{
    q->beginRemoveRows(QModelIndex(), start, end);
}

void QuerySession::Private::matchesRemoved()
{
    q->endRemoveRows();
}

void QuerySession::Private::matchesUpdated(int start, int end)
{
    emit q->dataChanged(q->createIndex(start, 0), q->createIndex(end, roleColumns.count()));
}

void QuerySession::Private::matchesArrived()
{
    //NOTE: this gets called from non-GUI threads
    matchesArrivedWhileExecuting = matchesArrivedWhileExecuting ||
                                   !executingMatches.isEmpty();
    if (!matchesArrivedWhileExecuting) {
        QMetaObject::invokeMethod(syncTimer, "startIfStopped");
    }
}

void QuerySession::Private::executionFinished(const QueryMatch &match, bool success)
{
    // remove the match from the list of matches being executed
    QMutableHashIterator<int, QueryMatch> it(executingMatches);
    while (it.hasNext()) {
        it.next();
        if (it.value() == match) {
            const int index = it.key();
            it.remove();
            matchesUpdated(index, index);
            break;
        }
    }

    // if we have no matches executing, check if there are matches waiting
    // synchronization
    if (executingMatches.isEmpty() && matchesArrivedWhileExecuting) {
        matchesArrivedWhileExecuting = false;
        matchesArrived();
    }
}

void QuerySession::Private::startMatchSynchronization()
{
    worker->syncMatches();
}

QuerySession::QuerySession(QObject *parent)
    : QAbstractItemModel(parent),
      d(new Private(this))
{
}

QuerySession::~QuerySession()
{
    d->workerThread->exit();
    delete d;
}

QAbstractItemModel *QuerySession::runnerModel() const
{
    return d->runnerModel;
}

void QuerySession::requestDefaultMatches()
{
    qDebug() << "Manager, default query:" << QThread::currentThread();
    d->worker->launchDefaultMatches();
    emit queryChanged(d->worker->query());
}

void QuerySession::requestMoreMatches()
{
    qDebug() << "Manager, more matches:" << QThread::currentThread();
    d->worker->launchMoreMatches();
}

void QuerySession::setQuery(const QString &query)
{
    qDebug() << "Manager:" << QThread::currentThread() << query;
    if (d->worker->launchQuery(query)) {
        emit queryChanged(d->worker->query());
    }
}

void QuerySession::setImageSize(const QSize &size)
{
    if (d->worker->setImageSize(size)) {
        emit imageSizeChanged(size);
    }
}

QSize QuerySession::imageSize() const
{
    return d->worker->imageSize();
}

void QuerySession::executeMatch(int index)
{
    QueryMatch match = d->worker->matchAt(index);

    if (!match.isValid()) {
        return;
    }

    if (match.isSearchTerm()) {
        setQuery(match.data().toString());
        return;
    }

    QHashIterator<int, QueryMatch> it(d->executingMatches);
    while (it.hasNext()) {
        it.next();
        if (it.value() == match) {
            return;
        }
    }

    d->executingMatches.insert(index, match);
    d->matchesUpdated(index, index);
    ExecRunnable *exec = new ExecRunnable(match);
    connect(exec, SIGNAL(finished(QueryMatch,bool)),
            this, SLOT(executionFinished(QueryMatch,bool)));
    QThreadPool::globalInstance()->start(exec);
}

void QuerySession::executeMatch(const QModelIndex &index)
{
    executeMatch(index.row());
}

void QuerySession::halt()
{
    d->matchesArrivedWhileExecuting = false;
    d->worker->endQuerySession();
}

QString QuerySession::query() const
{
    return d->worker->query();
}

int QuerySession::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return d->roles.count();
}

QVariant QuerySession::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.parent().isValid()) {
        return QVariant();
    }

    // QML and QWidget handle viewing models a bit differently
    // QML asks for a row and a role, basically ignoring roleColumns
    // Default QWidget views as for the DisplayRole of a row/column
    // so we adapt what data() returns based on what we are being asked for
    bool asText = false;
    if (index.column() > 0 &&
        index.column() < d->roleColumns.count() &&
        role == Qt::DisplayRole) {
        asText = true;
        role = d->roleColumns[index.column()];
    } else if (index.column() == d->imageRoleColumn && role == Qt::DecorationRole) {
        role = ImageRole;
    }

    // short circuit for execution; don't need the QueryMatch object
    if (role == ExecutingRole) {
        return d->executingMatches.contains(index.row());
    }

    QueryMatch match = d->worker->matchAt(index.row());

    switch (role) {
        case Qt::DisplayRole:
            return match.title();
            break;
        case TextRole:
            return match.text();
            break;
        case ImageRole:
            return match.image();
            break;
        case TypeRole:
            if (asText) {
                return textForEnum(this, "MatchType", match.type());
            } else {
                return match.type();
            }
            break;
        case SourceRole:
            if (asText) {
                return textForEnum(this, "MatchSource", match.source());
            } else {
                return match.source();
            }
            break;
        case PrecisionRole:
            if (asText) {
                return textForEnum(this, "MatchPrecision", match.precision());
            } else {
                return match.precision();
            }
            break;
        case UserDataRole:
            return match.userData();
            break;
        case DataRole:
            return match.data();
            break;
        case SearchTermRole:
            return match.isSearchTerm();
            break;
        case RunnerRole: {
            Runner *runner = match.runner();
            if (runner) {
                return runner->id();
            }
            break;
        }
        default:
            break;
    }

    return QVariant();
}

QVariant QuerySession::headerData(int section, Qt::Orientation orientation, int role) const {
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
            case ImageRole:
                return tr("Image");
                break;
            case TypeRole:
                return tr("Type");
                break;
            case SourceRole:
                return tr("From");
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
            case SearchTermRole:
                return tr("Search Term");
                break;
            case RunnerRole:
                return tr("Runner ID");
                break;
            case ExecutingRole:
                return tr("Executing");
            default:
                break;
        }
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}

QModelIndex QuerySession::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return QModelIndex();
    }

    return createIndex(row, column);
}

QModelIndex QuerySession::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

int QuerySession::rowCount(const QModelIndex & parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return d->worker->matchCount();
}

QHash<int, QByteArray> QuerySession::roleNames() const
{
    return d->roles;
}

} // namespace
#include "moc_querysession.cpp"
