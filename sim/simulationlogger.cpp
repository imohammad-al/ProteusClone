#include "simulationlogger.h"

SimulationLogger &SimulationLogger::instance()
{
    static SimulationLogger inst;
    return inst;
}

SimulationLogger::SimulationLogger(QObject *parent)
    : QObject(parent)
{
}

void SimulationLogger::log(LogLevel level, const QString &message)
{
    emit messageLogged(level, message);
}

void SimulationLogger::clear()
{
    emit cleared();
}
