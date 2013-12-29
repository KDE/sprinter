#ifndef RUNNERSESSIONDATA
#define RUNNERSESSIONDATA

#include <QVector>

#include "querymatch.h"

class AbstractRunner;

//TODO: use QExplicitlySharedDataPointer or QSharedDataPointer instead of doing ref counting manually?
class RunnerSessionData
{
public:
    enum MatchState {
        SynchronizedMatches,
        NewMatches
    };

    RunnerSessionData(AbstractRunner *runner);
    virtual ~RunnerSessionData();

    void addMatches(const QVector<QueryMatch> &matches);
    QVector<QueryMatch> matches(MatchState state) const;

    void ref();
    void deref();

private:
    class Private;
    Private * const d;
};

#endif