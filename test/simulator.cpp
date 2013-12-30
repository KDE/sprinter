/*
 * Copyright (C) 2013 Aaron Seigo <aseigo@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    m_queries << "date" << "time UTC" << "time MST" << "date Hong_Kong";
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

