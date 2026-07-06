#include "addcomponentcommand.h"

#include "../circuitscene.h"

#include <QGraphicsItem>

AddComponentCommand::AddComponentCommand(
    CircuitScene *scene,
    QGraphicsItem *item)
    : CommandBase(scene),
    m_item(item),
    m_inScene(false)
{
    setText("Add Component");
}

AddComponentCommand::~AddComponentCommand()
{
    if(!m_inScene && m_item)
        delete m_item;
}

void AddComponentCommand::redo()
{
    if(!m_scene || !m_item)
        return;

    if(!m_inScene)
    {
        m_scene->addItem(m_item);
        m_inScene = true;
    }
}

void AddComponentCommand::undo()
{
    if(!m_scene || !m_item)
        return;

    if(m_inScene)
    {
        m_scene->removeItem(m_item);
        m_inScene = false;
    }
}