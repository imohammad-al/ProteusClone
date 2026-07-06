#include "component.h"
#include "pin.h"

#include <QGraphicsScene>

int Component::s_nextId = 1;

Component::Component(QGraphicsItem *parent)
    : QGraphicsObject(parent),
    m_id(s_nextId++)
{
    setFlag(ItemIsMovable);
    setFlag(ItemIsSelectable);
    setFlag(ItemSendsGeometryChanges);
}

QVariant Component::itemChange(GraphicsItemChange change,
                               const QVariant &value)
{
    if(change == ItemPositionChange && scene())
    {
        QPointF p = value.toPointF();

        const int grid = 20;

        p.setX(qRound(p.x() / grid) * grid);
        p.setY(qRound(p.y() / grid) * grid);

        return p;
    }

    // وقتی حرکت یا چرخش کامل شد
    if(change == ItemPositionHasChanged ||
        change == ItemRotationHasChanged)
    {
        updateConnections();
    }

    return QGraphicsObject::itemChange(change, value);
}

int Component::id() const
{
    return m_id;
}

QString Component::reference() const
{
    return QString("R%1").arg(m_id);
}

QList<Pin*> Component::pins() const
{
    return m_pins;
}

void Component::addPin(Pin *pin)
{
    if(!m_pins.contains(pin))
        m_pins.append(pin);
}

// مهم: فقط یکی لازم است → حذف duplication
void Component::updateConnections()
{
    for(Pin *pin : m_pins)
    {
        if(pin)
            pin->updateWires();
    }
}