#include "querymatch.h"

#include "abstractrunner.h"

class QueryMatch::Private : public QSharedData
{
public:
    AbstractRunner *runner;
    QString text;
    Type type;
    Precision precision;
};

QueryMatch::QueryMatch(AbstractRunner *runner)
    : d(new Private)
{
    d->runner = runner;
    d->type = UnknownType;
    d->precision = UnrelatedMatch;
}

QueryMatch::QueryMatch(const QueryMatch &other)
    : d(other.d)
{
}

QueryMatch::~QueryMatch()
{
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

