#include "deletecomponentcommand.h"

#include "../circuitscene.h"

#include <QGraphicsItem>

DeleteComponentCommand::DeleteComponentCommand(
    CircuitScene *scene,
    QGraphicsItem *item)
    : CommandBase(scene),
    m_item(item),
    m_inScene(true)
{
    setText("Delete Component");
}

DeleteComponentCommand::~DeleteComponentCommand()
{
    if(!m_inScene && m_item)
        delete m_item;
}

void DeleteComponentCommand::redo()
{
    if(!m_scene || !m_item)
        return;

    if(m_inScene)
    {
        m_scene->removeItem(m_item);
        m_inScene = false;
    }
}

void DeleteComponentCommand::undo()
{
    if(!m_scene || !m_item)
        return;

    if(!m_inScene)
    {
        m_scene->addItem(m_item);
        m_inScene = true;
    }
}