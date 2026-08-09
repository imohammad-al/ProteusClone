#include "rotatecommand.h"

#include <QGraphicsItem>

RotateCommand::RotateCommand(
    CircuitScene *scene,
    QGraphicsItem *item,
    qreal angle)
    : CommandBase(scene),
    m_item(item)
{
    m_oldRotation = item->rotation();
    m_newRotation = m_oldRotation + angle;

    setText("Rotate Component");
}

void RotateCommand::redo()
{
    if(!m_item)
        return;

    m_item->setRotation(m_newRotation);

    // مهم: آپدیت سیم‌ها
    if(Component *c = dynamic_cast<Component*>(m_item))
    {
        c->updateConnections();
    }
}

void RotateCommand::undo()
{
    if(!m_item)
        return;

    m_item->setRotation(m_oldRotation);

    // مهم: آپدیت سیم‌ها
    if(Component *c = dynamic_cast<Component*>(m_item))
    {
        c->updateConnections();
    }
}