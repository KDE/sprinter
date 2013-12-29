#ifndef ABSTRACTRUNNER
#define ABSTRACTRUNNER

#include <QAtomicInt>
#include <QExplicitlySharedDataPointer>
#include <QObject>
#include <QRunnable>

#include <runnercontext.h>

class QueryMatch;
class AbstractRunner;
class RunnerContext;

class RunnerSessionData
{
public:
    virtual ~RunnerSessionData();

    void ref();
    void deref();

private:
    QAtomicInt m_ref;
};

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
