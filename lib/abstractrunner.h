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
    RunnableMatch(RunnerSessionData *sessionData);
    ~RunnableMatch();

    RunnerSessionData *sessionData();

    const QString query() const;
    bool isValid() const;
    void addMatches(const QList<QueryMatch> &matches);

    void setContext(const RunnerContext &context);
    RunnerContext &context() const;

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
    virtual RunnableMatch *createMatcher(RunnerSessionData *sessionData);

private:
    class Private;
    Private * const d;
};

#endif

