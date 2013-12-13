#ifndef ABSTRACTRUNNER
#define ABSTRACTRUNNER

#include <QExplicitlySharedDataPointer>
#include <QObject>
#include <QRunnable>

class QueryMatch;
class AbstractRunner;

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

class QueryMatch
{
public:
    QueryMatch(AbstractRunner *runner);
    QueryMatch(const QueryMatch &other);
    ~QueryMatch();

    void setText(const QString &text);
    QString text() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

#endif

