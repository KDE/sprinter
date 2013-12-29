#include "querymatch.h"

#include "abstractrunner.h"

class QueryMatch::Private : public QSharedData
{
public:
    Private(AbstractRunner *r)
        : runner(r),
          type(UnknownType),
          precision(UnrelatedMatch),
          updateInterval(0)
    {
    }

    AbstractRunner *runner;
    QString title;
    QString text;
    QString id;
    Type type;
    Precision precision;
    uint updateInterval;
};

QueryMatch::QueryMatch()
    : d(new Private(0))
{
}

//FIXME: if the runner is deleted?
QueryMatch::QueryMatch(AbstractRunner *runner)
    : d(new Private(runner))
{
}

QueryMatch::QueryMatch(const QueryMatch &other)
    : d(other.d)
{
}

QueryMatch::~QueryMatch()
{
}

QueryMatch &QueryMatch::operator=(const QueryMatch &other)
{
    d = other.d;
    return *this;
}

void QueryMatch::setTitle(const QString &title)
{
    d->title = title;
}

QString QueryMatch::title() const
{
    return d->title;
}

void QueryMatch::setText(const QString &text)
{
    d->text = text;
}

QString QueryMatch::text() const
{
    return d->text;
}

void QueryMatch::setType(Type type)
{
    d->type = type;
}

QueryMatch::Type QueryMatch::type() const
{
    return d->type;
}

void QueryMatch::setPrecision(Precision precision)
{
    d->precision = precision;
}

QueryMatch::Precision QueryMatch::precision() const
{
    return d->precision;
}

void QueryMatch::setInternalId(const QString &id)
{
    d->id = id;
}

QString QueryMatch::internalId() const
{
    return d->id;
}

void QueryMatch::setUpdateInterval(uint interval)
{
    d->updateInterval = interval;
    //TODO: implement interval checking
    if (interval > 0) {

    } else {

    }
}

uint QueryMatch::updateInterval() const
{
    return d->updateInterval;
}
