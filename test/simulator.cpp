#include "simulator.h"

#include <QDebug>
#include <QTimer>

Simulator::Simulator(QObject * parent)
    : QObject(parent),
      m_timer(new QTimer(this)),
      m_pos(0)
{
    connect(m_timer, SIGNAL(timeout()), this, SLOT(generateQuery()));
    m_timer->setSingleShot(true);
    m_queries << "KDE" << "plasma" << "earthworks";
    m_iterator = m_queries.constBegin();
    setupTimer();
}

void Simulator::setupTimer()
{
    ++m_pos;
    bool fast = true;

    if (m_pos > (*m_iterator).length()) {
        // at the end of the string, grab the next one

        ++m_iterator;
        if (m_iterator == m_queries.constEnd()) {
            emit done();
            return;
        } else {
            fast = false;
            m_pos = 1;
        }
    }

    const int interval = fast ? 50 * (qreal(qrand()) / RAND_MAX)
                              : 500;
    m_timer->start(interval);
}

void Simulator::generateQuery()
{
    emit query((*m_iterator).left(m_pos));
    setupTimer();
}

#include <moc_simulator.cpp>

