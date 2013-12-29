#include "abstractrunner.h"

#include <QDebug>
#include <QSharedData>
#include <QWeakPointer>

RunnerSessionData::~RunnerSessionData()
{
}

void RunnerSessionData::ref()
{
    m_ref.ref();
}

void RunnerSessionData::deref()
{
    if (!m_ref.deref()) {
        delete this;
    }
}

class AbstractRunner::Private
{
public:
    Private()
        : minQueryLength(3)
    {
    }

    uint minQueryLength;
};

AbstractRunner::AbstractRunner(QObject *parent)
    : QObject(parent),
      d(new Private)
{
}

AbstractRunner::~AbstractRunner()
{
    delete d;
}

void AbstractRunner::setMinQueryLength(uint length)
{
    d->minQueryLength = length;
}

int AbstractRunner::minQueryLength() const
{
    return d->minQueryLength;
}

RunnerSessionData *AbstractRunner::createSessionData()
{
    return new RunnerSessionData();
}

bool AbstractRunner::shouldStartMatch(const RunnerSessionData *sessionData, const RunnerContext &context) const
{
    if (!sessionData) {
        return false;
    }

    if (!context.isValid()) {
        return false;
    }

    if ((uint)context.query().length() < d->minQueryLength) {
        return false;
    }

    return true;
}

void AbstractRunner::startMatch(RunnerSessionData *sessionData, RunnerContext &context)
{
    if (shouldStartMatch(sessionData, context)) {
        match(sessionData, context);
    }
}

void AbstractRunner::match(RunnerSessionData *sessionData, RunnerContext &context)
{
    Q_UNUSED(sessionData)
    Q_UNUSED(context)
}

#include "abstractrunner.moc"

