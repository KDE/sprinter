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
