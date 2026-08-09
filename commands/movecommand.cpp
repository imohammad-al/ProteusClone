#include "movecommand.h"

#include <QGraphicsItem>

MoveCommand::MoveCommand(
    CircuitScene *scene,
    const QList<QGraphicsItem*> &items)
    : CommandBase(scene)
{
    setText("Move Component");

    for(QGraphicsItem *item : items)
    {
        m_oldPos[item] = item->pos();
    }
}

void MoveCommand::setNewPositions(
    const QMap<QGraphicsItem*, QPointF> &newPos)
{
    m_newPos = newPos;
}

void MoveCommand::redo()
{
    if(m_firstRedo)
    {
        m_firstRedo = false;
        return;
    }

    for(auto it = m_newPos.begin(); it != m_newPos.end(); ++it)
    {
        if(it.key())
            it.key()->setPos(it.value());
    }
}

void MoveCommand::undo()
{
    for(auto it = m_oldPos.begin(); it != m_oldPos.end(); ++it)
    {
        if(it.key())
            it.key()->setPos(it.value());
    }
}