#include "addwirecommand.h"

#include "../circuitscene.h"
#include "../wire.h"

AddWireCommand::AddWireCommand(CircuitScene *scene,
                               Wire *wire,
                               QUndoCommand *parent)
    : QUndoCommand(parent),
    m_scene(scene),
    m_wire(wire)
{
    setText("Add Wire");
}

void AddWireCommand::redo()
{
    if(m_firstRedo)
    {
        m_firstRedo = false;
        return;
    }

    if(m_wire && m_scene)
        m_scene->addItem(m_wire);
}

void AddWireCommand::undo()
{
    if(m_wire && m_scene)
        m_scene->removeItem(m_wire);
}