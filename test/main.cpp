#include <QCoreApplication>

#include "runnermanager.h"
#include "simulator.h"

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    Simulator *simulator = new Simulator;
    RunnerManager *manager = new RunnerManager;

    QObject::connect(simulator, SIGNAL(query(QString)), manager, SLOT(setQuery(QString)));
    //QObject::connect(simulator, SIGNAL(done()), manager, SIGNAL(querySessionCompleted()));
    QObject::connect(simulator, SIGNAL(done()), manager, SLOT(deleteLater()));
    QObject::connect(manager, SIGNAL(destroyed()), &app, SLOT(quit()));

    return app.exec();
}
