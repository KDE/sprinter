#ifndef ABSTRACTRUNNER
#define ABSTRACTRUNNER

#include <QExplicitlySharedDataPointer>
#include <QObject>
#include <QRunnable>

#include <runnercontext.h>

class QueryMatch;
class AbstractRunner;
class RunnerContext;

class RunnerSessionData
{
};

class RunnableMatch : public QRunnable
{
public:
    RunnableMatch(RunnerSessionData *sessionData, const RunnerContext &context);
    ~RunnableMatch();

    RunnerSessionData *sessionData();

    const QString query() const;
    bool isValid() const;
    void addMatches(const QList<QueryMatch> &matches);

    virtual void match() = 0;

private:
    void run();

    class Private;
    Private * const d;
};

class AbstractRunner : public QObject
{
    Q_OBJECT

public:
    AbstractRunner(QObject *parent);
    ~AbstractRunner();

    virtual RunnerSessionData *createSessionData();
    virtual RunnableMatch *createMatcher(RunnerSessionData *sessionData, RunnerContext &context);

private:
    class Private;
    Private * const d;
};

#endif

