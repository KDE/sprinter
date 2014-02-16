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
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSplitter>
#include <QTreeView>

#include "sprinter/querysession.h"

using namespace Sprinter;

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    QuerySession *manager = new QuerySession;

    QSplitter *splitter = new QSplitter;

    QWidget *top = new QWidget(splitter);

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

    QPushButton *defaultMatchButton = new QPushButton(top);
    defaultMatchButton->setText("Default matches");
    QObject::connect(defaultMatchButton, SIGNAL(clicked()),
                     manager, SLOT(requestDefaultMatches()));

    QPushButton *moreMatchesButton = new QPushButton(top);
    moreMatchesButton->setText("More matches");
    QObject::connect(moreMatchesButton, SIGNAL(clicked()),
                     manager, SLOT(requestMoreMatches()));

    QGridLayout *matchLayout = new QGridLayout(top);
    matchLayout->addWidget(edit, 0, 0, 1, 2);
    matchLayout->addWidget(matchView, 1, 0, 1, 2);
    matchLayout->addWidget(defaultMatchButton, 2, 0);
    matchLayout->addWidget(moreMatchesButton, 2, 1);
    splitter->addWidget(top);

    top = new QWidget(splitter);
    QVBoxLayout *runnerLayout = new QVBoxLayout(top);
    runnerLayout->addWidget(new QLabel("Runners"));
    QTreeView *runnerView = new QTreeView(top);
    runnerView->setModel(manager->runnerModel());
    runnerView->setAllColumnsShowFocus(true);
    QObject::connect(runnerView, SIGNAL(doubleClicked(QModelIndex)),
                     manager->runnerModel(), SLOT(loadRunner(QModelIndex)));
    runnerLayout->addWidget(runnerView);
    splitter->addWidget(top);

    QAction *action = new QAction(top);
    action->setShortcut(Qt::CTRL + Qt::Key_Q);
    top->addAction(action);
    QObject::connect(action, SIGNAL(triggered()), &app, SLOT(quit()));

    action = new QAction(top);
    action->setShortcut(Qt::Key_Escape);
    top->addAction(action);
    QObject::connect(action, SIGNAL(triggered()), manager, SLOT(halt()));

    splitter->resize(1000, 700);
    splitter->show();
    edit->setFocus();

    return app.exec();
}
