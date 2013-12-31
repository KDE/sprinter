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

#include <QApplication>
#include <QTimer>

#include <QAction>
#include <QLineEdit>
#include <QListView>
#include <QVBoxLayout>

#include "runnermanager.h"
#include "simulator.h"

// #define SIMULATE

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    Simulator *simulator = new Simulator;
    RunnerManager *manager = new RunnerManager;

#ifdef SIMULATE
    QObject::connect(simulator, SIGNAL(query(QString)), manager, SLOT(setQuery(QString)));
    //QObject::connect(simulator, SIGNAL(done()), manager, SIGNAL(querySessionCompleted()));
    QTimer delayedQuit;
    delayedQuit.setInterval(1000);
    QObject::connect(simulator, SIGNAL(done()), &delayedQuit, SLOT(start()));
    QObject::connect(&delayedQuit, SIGNAL(timeout()), manager, SLOT(deleteLater()));
    QObject::connect(manager, SIGNAL(destroyed()), &app, SLOT(quit()));
#else
    QWidget *top = new QWidget;

    QLineEdit *edit = new QLineEdit(top);
    QObject::connect(edit, SIGNAL(textChanged(QString)),
                     manager, SLOT(setQuery(QString)));

    QListView *view = new QListView(top);
    view->setModel(manager);

    QVBoxLayout *layout = new QVBoxLayout(top);
    layout->addWidget(edit);
    layout->addWidget(view);
    top->resize(400, 700);
    top->show();
    edit->setFocus();

    QAction *action = new QAction(top);
    action->setShortcut(Qt::CTRL + Qt::Key_Q);
    top->addAction(action);
    QObject::connect(action, SIGNAL(triggered()), &app, SLOT(quit()));
#endif


    return app.exec();
}
