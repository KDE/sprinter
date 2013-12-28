#include "querymatch.h"

#include "abstractrunner.h"

class QueryMatch::Private : public QSharedData
{
public:
    AbstractRunner *runner;
    QString text;
};

QueryMatch::QueryMatch(AbstractRunner *runner)
    : d(new Private)
{
    d->runner = runner;
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
