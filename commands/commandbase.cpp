#include "commandbase.h"

#include "../circuitscene.h"

CommandBase::CommandBase(
    CircuitScene *scene,
    QUndoCommand *parent)
    : QUndoCommand(parent),
    m_scene(scene)
{
}