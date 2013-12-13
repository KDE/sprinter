#include "runnermanagerthread.h"

#include <QDebug>

#include "runnerhelper.h"
#include "runnermanager.h"

class RunnerManagerThread::Private
{
public:
    Private(RunnerManagerThread *thread, RunnerManager *m)
        : q(thread),
          manager(m)
    {

    }

    ~Private();
    RunnerManagerThread *q;
    RunnerManager *manager;
};

RunnerManagerThread::Private::~Private()
{
}

RunnerManagerThread::RunnerManagerThread(RunnerManager *parent)
    : QThread(parent),
      d(new Private(this, parent))
{
}

RunnerManagerThread::~RunnerManagerThread()
{
    delete d;
}

void RunnerManagerThread::run()
{
    RunnerHelper r;
    connect(d->manager, SIGNAL(queryChanged(QString)), &r, SLOT(startQuery(QString)));
    qDebug() << "should we do this query, then?" << d->manager->query();
    r.startQuery(d->manager->query());
    exec();
    deleteLater();
    qDebug() << "leaving run";
}

#include "runnermanagerthread.moc"

