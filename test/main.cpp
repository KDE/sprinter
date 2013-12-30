#include <QCoreApplication>
#include <QTimer>

#include "runnermanager.h"
#include "simulator.h"

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Simulator *simulator = new Simulator;
    RunnerManager *manager = new RunnerManager;

    QObject::connect(simulator, SIGNAL(query(QString)), manager, SLOT(setQuery(QString)));
    //QObject::connect(simulator, SIGNAL(done()), manager, SIGNAL(querySessionCompleted()));
    QTimer delayedQuit;
    delayedQuit.setInterval(100);
    QObject::connect(simulator, SIGNAL(done()), &delayedQuit, SLOT(start()));
    QObject::connect(&delayedQuit, SIGNAL(timeout()), manager, SLOT(deleteLater()));
    QObject::connect(manager, SIGNAL(destroyed()), &app, SLOT(quit()));

    return app.exec();
}
