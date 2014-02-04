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
#include <QPushButton>
#include <QTreeView>
#include <QGridLayout>

#include "querysession.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QuerySession *manager = new QuerySession;

    QWidget *top = new QWidget;

    QLineEdit *edit = new QLineEdit(top);
    edit->setPlaceholderText("Enter search term");
    QObject::connect(edit, SIGNAL(textChanged(QString)),
                     manager, SLOT(setQuery(QString)));
    QObject::connect(manager, SIGNAL(queryChanged(QString)),
                     edit, SLOT(setText(QString)));
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

    QPushButton *defaultMatchButton = new QPushButton(top);
    defaultMatchButton->setText("Default matches");
    QObject::connect(defaultMatchButton, SIGNAL(clicked()),
                     manager, SLOT(requestDefaultMatches()));

    QGridLayout *topLayout = new QGridLayout(top);
    topLayout->addWidget(edit, 0, 0);
    topLayout->addWidget(matchView, 1, 0);
    topLayout->addWidget(defaultMatchButton, 2, 0);
    topLayout->addWidget(new QLabel("Runners"), 0, 1);
    topLayout->addWidget(runnerView, 1, 1);

    QAction *action = new QAction(top);
    action->setShortcut(Qt::CTRL + Qt::Key_Q);
    top->addAction(action);
    QObject::connect(action, SIGNAL(triggered()), &app, SLOT(quit()));

    action = new QAction(top);
    action->setShortcut(Qt::Key_Escape);
    top->addAction(action);
    QObject::connect(action, SIGNAL(triggered()), manager, SLOT(halt()));

    top->resize(1000, 700);
    top->show();
    edit->setFocus();

    return app.exec();
}
