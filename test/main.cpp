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

#include <QApplication>
#include <QTimer>

#include <QAction>
#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QTreeView>
#include <QGridLayout>

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
    edit->setPlaceholderText("Enter search term");
    QObject::connect(edit, SIGNAL(textChanged(QString)),
                     manager, SLOT(setQuery(QString)));

    QTreeView *matchView = new QTreeView(top);
    matchView->setModel(manager);
    matchView->setAllColumnsShowFocus(true);
    QObject::connect(matchView, SIGNAL(doubleClicked(QModelIndex)),
                     manager, SLOT(executeMatch(QModelIndex)));



    QTreeView *runnerView = new QTreeView(top);
    runnerView->setModel(manager->runnerModel());
    runnerView->setAllColumnsShowFocus(true);
    QObject::connect(runnerView, SIGNAL(doubleClicked(QModelIndex)),
                     manager->runnerModel(), SLOT(loadRunner(QModelIndex)));

    QGridLayout *topLayout = new QGridLayout(top);
    topLayout->addWidget(edit, 0, 0);
    topLayout->addWidget(matchView, 1, 0);
    topLayout->addWidget(new QLabel("Runners"), 0, 1);
    topLayout->addWidget(runnerView, 1, 1);

    QAction *action = new QAction(top);
    action->setShortcut(Qt::CTRL + Qt::Key_Q);
    top->addAction(action);
    QObject::connect(action, SIGNAL(triggered()), &app, SLOT(quit()));
#endif

    top->resize(800, 700);
    top->show();
    edit->setFocus();

    return app.exec();
}
