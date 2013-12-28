
#include "runnercontext.h"


#include <QDebug>
#include <QReadWriteLock>

class RunnerContext::Private : public QSharedData
{
public:
    Private(RunnerContext *context)
        : QSharedData(),
          q(context)
    {
    }


    Private(const Private &p)
        : QSharedData(),
          q(p.q)
    {
        //kDebug() << "¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿boo yeah" <<
        //type;
    }

    QString query;
    QReadWriteLock lock;
    RunnerContext *q;
};

RunnerContext::RunnerContext()
    : d(new Private(this))
{
}

RunnerContext::RunnerContext(const RunnerContext &other)
{
    other.d->lock.lockForRead();
    d = other.d;
    other.d->lock.unlock();
}

RunnerContext::~RunnerContext()
{
}

RunnerContext &RunnerContext::operator=(const RunnerContext &other)
{
    if (this->d == other.d) {
        return *this;
    }

    QExplicitlySharedDataPointer<RunnerContext::Private> oldD = d;
    d->lock.lockForWrite();
    other.d->lock.lockForRead();
    d = other.d;
    other.d->lock.unlock();
    oldD->lock.unlock();
    return *this;
}

void RunnerContext::reset()
{
    // We will detach if we are a copy of someone. But we will reset
    // if we are the 'main' context others copied from. Resetting
    // one RunnerContext makes all the copies obsolete.

    // We need to mark the q pointer of the detached RunnerContextPrivate
    // as dirty on detach to avoid receiving results for old queries
    d->q = 0;

    d.detach();
    d->query.clear();

    // Now that we detached the d pointer we need to reset its q pointer
    d->q = this;
}

void RunnerContext::setQuery(const QString &query)
{
    d->query = query;
}

QString RunnerContext::query() const
{
    return d->query;
}

bool RunnerContext::isValid() const
{
    return d->q;
}

void RunnerContext::addMatches(const QList<QueryMatch> &matches)
{
    if (!d->q) {
        return;
    }

    qDebug() << "Got new matches ..." << matches.count();
}
