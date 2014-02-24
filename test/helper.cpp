/*
 * Copyright (C) 2014 Aaron Seigo <aseigo@kde.org>
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

#include "helper.h"

#include <QDebug>

Helper::Helper(QObject *parent)
    : QObject(parent),
      m_session(new Sprinter::QuerySession(this))
{
}

Helper::~Helper()
{

}

Sprinter::QuerySession *Helper::session()
{
    return m_session;
}

void Helper::loadAllRunners()
{
    qDebug() << "loading all runnears";
    const int numRunners = m_session->runnerModel()->rowCount();
    for (int i = 0; i < numRunners; ++i) {
        QMetaObject::invokeMethod(m_session->runnerModel(),
                                  "loadRunner",
                                  Q_ARG(int, i));
    }
}

#include "moc_helper.cpp"
