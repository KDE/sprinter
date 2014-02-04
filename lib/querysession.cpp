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

#include "abstractrunner.h"
#include "querysessionthread_p.h"
#include "runnermodel_p.h"

QuerySession::Private::Private(QuerySession *session)
    : q(session),
      thread(new QuerySessionThread(session)),
      runnerModel(new RunnerModel(thread, session)),
      matchesArrivedWhileExecuting(false)
{
    qRegisterMetaType<QUuid>("QUuid");
    qRegisterMetaType<QUuid>("QueryContext");
    qRegisterMetaType<QUuid>("QueryMatch");

    q->connect(thread, SIGNAL(resetModel()), q, SLOT(resetModel()));
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
    matchesArrivedWhileExecuting = matchesArrivedWhileExecuting ||
                                   !executingMatches.isEmpty();
    if (!matchesArrivedWhileExecuting) {
        thread->syncMatches();
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

QuerySession::QuerySession(QObject *parent)
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

    d->thread->start();
}

QuerySession::~QuerySession()
{
    d->thread->exit();
    delete d;
}

QAbstractItemModel *QuerySession::runnerModel() const
{
    return d->runnerModel;
}

void QuerySession::requestDefaultMatches()
{
    qDebug() << "Manager, default query:" << QThread::currentThread();
    d->thread->launchDefaultMatches();
    emit queryChanged(d->thread->query());
}

void QuerySession::setQuery(const QString &query)
{
    qDebug() << "Manager:" << QThread::currentThread() << query;
    if (d->thread->launchQuery(query)) {
        emit queryChanged(d->thread->query());
    }
}

void QuerySession::executeMatch(int index)
{
    QueryMatch match = d->thread->matchAt(index);

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
    emit d->thread->requestEndQuerySession();
}

QString QuerySession::query() const
{
    return d->thread->query();
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
    }

    // short circuit for execution; don't need the QueryMatch object
    if (role == ExecutingRole) {
        return d->executingMatches.contains(index.row());
    }

    QueryMatch match = d->thread->matchAt(index.row());

    switch (role) {
        case Qt::DisplayRole:
            return match.title();
            break;
        case TextRole:
            return match.text();
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
            AbstractRunner *runner = match.runner();
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

    return d->thread->matchCount();
}

QHash<int, QByteArray> QuerySession::roleNames() const
{
    return d->roles;
}

#include "moc_querysession.cpp"
