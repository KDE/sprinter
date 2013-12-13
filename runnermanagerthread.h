#ifndef RUNNERMANAGERTHREAD
#define RUNNERMANAGERTHREAD

#include <QThread>

class RunnerManager;

class RunnerManagerThread : public QThread
{
    Q_OBJECT

public:
    RunnerManagerThread(RunnerManager *parent = 0);
    ~RunnerManagerThread();

    void run();

Q_SIGNALS:
    void matchesUpdated();

private:
    class Private;
    Private * const d;
};

#endif

