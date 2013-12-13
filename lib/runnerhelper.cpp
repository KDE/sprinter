#include "runnerhelper.h"

#include <QDebug>
#include <QThread>
#include <QThreadPool>

// temporary include for non-pluggable plugins
#include "runners.h"

RunnerHelper::RunnerHelper(QObject *parent)
    : QObject(parent)
{
    loadRunners();
    retrieveSessionData();
}

RunnerHelper::~RunnerHelper()
{
    QHashIterator<AbstractRunner *, RunnerSessionData *> sessions(m_runnerSessions);
    while (sessions.hasNext()) {
        sessions.next();
        delete sessions.value();
    }

    qDeleteAll(m_runners);
}

void RunnerHelper::loadRunners()
{
    if (!m_runners.isEmpty()) {
        return;
    }

    //TODO: this should be loading from plugins, obviously
    m_runners.insert(new RunnerA);
    m_runners.insert(new RunnerB);
    m_runners.insert(new RunnerC);
    m_runners.insert(new RunnerD);
    qDebug() << "Runners are loaded" << m_runners.count();
}

void RunnerHelper::retrieveSessionData()
{
    QSetIterator<AbstractRunner *> it(m_runners);
    while (it.hasNext()) {
        AbstractRunner *runner = it.next();

        if (m_runnerSessions.contains(runner)) {
            continue;
        }

        SessionDataRetriever *rtrver = new SessionDataRetriever(runner);
        rtrver->setAutoDelete(true);
        connect(rtrver, SIGNAL(sessionDataRetrieved(AbstractRunner*,RunnerSessionData*)),
                this, SLOT(sessionDataRetrieved(AbstractRunner*,RunnerSessionData*)));
        QThreadPool::globalInstance()->start(rtrver);
    }
}

void RunnerHelper::launchJobs()
{
    QHashIterator<AbstractRunner *, RunnerSessionData *> sessions(m_runnerSessions);
    while (sessions.hasNext()) {
        sessions.next();
        AbstractRunner *runner = sessions.key();
        RunnableMatch *match = m_matchers.value(runner);
        if (!match) {
            // no matcher, so we fetch the match for it
            //TODO: also thread?
            match = runner->createMatcher(sessions.value(), m_context);
        }

        if (match) {
            QThreadPool::globalInstance()->start(match);
        }
    }
}

void RunnerHelper::sessionDataRetrieved(AbstractRunner *runner, RunnerSessionData *data)
{
    qDebug() << "got the data for" << runner;
    if (!runner) {
        delete data;
        return;
    }

    delete m_runnerSessions.value(runner);
    m_runnerSessions.insert(runner, data);
    //m_activeRunners.insert(runner);
    launchJobs();
}

void RunnerHelper::startQuery(const QString &query)
{
    if (query.isEmpty() || m_query == query) {
        return;
    }

    qDebug() << "kicking off a query ..." << QThread::currentThread() << query;
    m_query = query;
    m_context.setQuery(m_query);
    launchJobs();
}

void RunnerHelper::querySessionCompleted()
{
    QMutableHashIterator<AbstractRunner *, RunnerSessionData *> it(m_runnerSessions);
    while (it.hasNext()) {
        it.next();
        delete it.value();
        it.value() = 0;
    }
    m_runnerSessions.clear();
}

SessionDataRetriever::SessionDataRetriever(AbstractRunner *runner)
    : m_runner(runner)
{
}

void SessionDataRetriever::run()
{
    if (m_runner) {
        //FIXME: race condition between check value and using it below
        RunnerSessionData *session = m_runner.data()->createSessionData();
        emit sessionDataRetrieved(m_runner.data(), session);
    }
}

#include "runnerhelper.moc"

