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

#ifndef SIMULATOR
#define SIMULATOR

#include <QObject>
#include <QStringList>

class QTimer;

class Simulator : public QObject
{
    Q_OBJECT

public:
    Simulator(QObject * parent = 0);

Q_SIGNALS:
    void query(const QString &query);
    void done();

private Q_SLOTS:
    void generateQuery();

private:
    void setupTimer();

    QStringList m_queries;
    QStringList::const_iterator m_iterator;
    QTimer *m_timer;
    int m_pos;
};

#endif

