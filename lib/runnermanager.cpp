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
#include "runnermanager_p.h"

#include <QDebug>
#include <QMetaEnum>
#include <QThreadPool>

#include "abstractrunner.h"
#include "runnermanagerthread_p.h"
#include "runnermodel_p.h"

RunnerManager::Private::Private(RunnerManager *manager)
    : q(manager),
      thread(new RunnerManagerThread(manager)),
      runnerModel(new RunnerModel(thread, manager)),
      matchesArrivedWhileExecuting(false)
{
    qRegisterMetaType<QUuid>("QUuid");
    qRegisterMetaType<QUuid>("QueryContext");
    qRegisterMetaType<QUuid>("QueryMatch");

    q->connect(thread, SIGNAL(resetModel()), q, SLOT(resetModel()));
}

void RunnerManager::Private::resetModel()
{
    q->beginResetModel();
    q->endResetModel();
}

void RunnerManager::Private::addingMatches(int start, int end)
{
    q->beginInsertRows(QModelIndex(), start, end);
}

void RunnerManager::Private::matchesAdded()
{
    q->endInsertRows();
}

void RunnerManager::Private::removingMatches(int start, int end)
{
    q->beginRemoveRows(QModelIndex(), start, end);
}

void RunnerManager::Private::matchesRemoved()
{
    q->endRemoveRows();
}

void RunnerManager::Private::matchesUpdated(int start, int end)
{
    emit q->dataChanged(q->createIndex(start, 0), q->createIndex(end, 0));
}

void RunnerManager::Private::matchesArrived()
{
    matchesArrivedWhileExecuting = matchesArrivedWhileExecuting ||
                                   !executingMatches.isEmpty();
    if (!matchesArrivedWhileExecuting) {
        thread->syncMatches();
    }
}

void RunnerManager::Private::executionFinished(const QueryMatch &match, bool success)
{
    // remove the match from the list of matches being executed
    int index = -1;
    for (int i = 0; i < executingMatches.count(); ++i) {
        if (executingMatches[i].first == match) {
            index = i;
            executingMatches.remove(i);
            break;
        }
    }

    // now emit the signal; the application may elect to end the session here
    emit q->executionFinished(index, success);

    // if we have no matches executing, check if there are matches waiting
    // synchronization
    if (executingMatches.isEmpty() && matchesArrivedWhileExecuting) {
        matchesArrivedWhileExecuting = false;
        matchesArrived();
    }
}

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

    d->thread->start();
}

RunnerManager::~RunnerManager()
{
    d->thread->exit();
    delete d;
}

QAbstractItemModel *RunnerManager::runnerModel() const
{
    return d->runnerModel;
}

void RunnerManager::setQuery(const QString &query)
{
    qDebug() << "Manager:" << QThread::currentThread() << query;
    d->query = query;
    //QMetaObject::invokeMethod(d->thread, "startQuery", Qt::AutoConnection, Q_ARG(QString, query));
    emit queryChanged(query);
}

void RunnerManager::executeMatch(int index)
{
    QueryMatch match = d->thread->matchAt(index);

    if (!match.isValid()) {
        return;
    }

    if (match.isSearchTerm()) {
        setQuery(match.data().toString());
        return;
    }

    for (int i = 0; i < d->executingMatches.count(); ++i) {
        if (d->executingMatches[i].first == match) {
            return;
        }
    }

    d->executingMatches.append(qMakePair<QueryMatch, int>(match, index));
    emit executionStarted(index);
    ExecRunnable *exec = new ExecRunnable(match);
    connect(exec, SIGNAL(finished(QueryMatch,bool)),
            this, SLOT(executionFinished(QueryMatch,bool)));
    QThreadPool::globalInstance()->start(exec);
}

void RunnerManager::executeMatch(const QModelIndex &index)
{
    executeMatch(index.row());
}

void RunnerManager::endQuerySession()
{
    d->matchesArrivedWhileExecuting = false;
    emit d->thread->requestEndQuerySession();
}

QString RunnerManager::query() const
{
    return d->query;
}

int RunnerManager::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return d->roles.count();
}

QString textForEnum(const QObject *obj, const char *enumName, int value)
{
    QMetaEnum e = obj->metaObject()->enumerator(obj->metaObject()->indexOfEnumerator(enumName));
    for (int i = 0; i < e.keyCount(); ++i) {
        if (e.value(i) == value) {
            return e.key(i);
        }
    }

    return "Unknown";
}

QVariant RunnerManager::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.parent().isValid()) {
        return QVariant();
    }

    QueryMatch match = d->thread->matchAt(index.row());

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
    if (parent.isValid()) {
        return 0;
    }

    return d->thread->matchCount();
}

QHash<int, QByteArray> RunnerManager::roleNames() const
{
    return d->roles;
}

#include "moc_runnermanager.cpp"
