#ifndef RUNNERCONTEXT
#define RUNNERCONTEXT

#include <querymatch.h>

#include <QExplicitlySharedDataPointer>
#include <QString>

class QueryMatch;


class RunnerContext
{
public:
    RunnerContext();
    RunnerContext(const RunnerContext &other);
    ~RunnerContext();

    RunnerContext &operator=(const RunnerContext &other);

    void setQuery(const QString &query);
    QString query() const;

    bool isValid() const;

    void addMatches(const QList<QueryMatch> &matches);

    void reset();

private:
    class Private;
    QExplicitlySharedDataPointer<Private> d;
};

#endif
