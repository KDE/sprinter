/*
 * Copyright (C) 2013 Aaron Seigo <aseigo@kde.org>
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
};

RunnerManager::RunnerManager(QObject *parent)
    : QAbstractItemModel(parent),
      d(new Private(this))
{
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

QString RunnerManager::query() const
{
    return d->query;
}

int RunnerManager::columnCount(const QModelIndex &parent) const
{
    //TODO
    Q_UNUSED(parent)
    return 0;
}

QVariant RunnerManager::data(const QModelIndex &index, int role) const
{
    //TODO
    Q_UNUSED(index)
    Q_UNUSED(role)
    return QVariant();
}

QModelIndex RunnerManager::index(int row, int column, const QModelIndex &parent) const
{
    //TODO
    Q_UNUSED(row)
    Q_UNUSED(column)
    Q_UNUSED(parent)
    return QModelIndex();
}

QModelIndex RunnerManager::parent(const QModelIndex &index) const
{
    //TODO
    Q_UNUSED(index)
    return QModelIndex();
}

int RunnerManager::rowCount(const QModelIndex & parent) const
{
    //TODO
    Q_UNUSED(parent)
    return 0;
}

#include "moc_runnermanager.cpp"
