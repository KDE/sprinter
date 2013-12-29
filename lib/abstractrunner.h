#ifndef ABSTRACTRUNNER
#define ABSTRACTRUNNER

#include <QObject>
#include <QRunnable>

#include <runnercontext.h>
#include <runnersessiondata.h>

class QueryMatch;
class AbstractRunner;
class RunnerContext;
class RunnerSessionData;

class AbstractRunner : public QObject
{
    Q_OBJECT

public:
    AbstractRunner(QObject *parent);
    ~AbstractRunner();

    bool shouldStartMatch(const RunnerSessionData *sessionData, const RunnerContext &context) const;

    void startMatch(RunnerSessionData *sessionData, RunnerContext &context);
    virtual RunnerSessionData *createSessionData();

    void setMinQueryLength(uint length);
    int minQueryLength() const;

protected:
    virtual void match(RunnerSessionData *sessionData, RunnerContext &context);

private:
    class Private;
    Private * const d;
};

#endif
