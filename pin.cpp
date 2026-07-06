#include "pin.h"
#include "wire.h"
#include "node.h"
#include "net.h"
#include <QGraphicsSceneMouseEvent>

Pin::Pin(QGraphicsItem *parent)
    : QGraphicsObject(parent)
{
    setAcceptedMouseButtons(Qt::LeftButton);

    setAcceptHoverEvents(true);

    setFlag(ItemSendsScenePositionChanges);
}

QRectF Pin::boundingRect() const
{
    return QRectF(-4,-4,8,8);
}

void Pin::paint(QPainter *painter,
                const QStyleOptionGraphicsItem *,
                QWidget *)
{
    painter->setPen(Qt::NoPen);

    painter->setBrush(Qt::red);

    painter->drawEllipse(boundingRect());
}

void Pin::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsObject::mousePressEvent(event);
}

QVariant Pin::itemChange(GraphicsItemChange change,
                         const QVariant &value)
{
    if(change == ItemScenePositionHasChanged)
    {
        updateWires();
    }

    return QGraphicsObject::itemChange(change,value);
}

void Pin::addWire(Wire *wire)
{
    if(!m_wires.contains(wire))
        m_wires.append(wire);
}

void Pin::updateWires()
{
    for(Wire *wire : m_wires)
    {
        if(wire)
            wire->updatePosition();
    }
}
Node *Pin::node() const
{
    return m_node;
}

void Pin::setNode(Node *node)
{
    // اگر تغییری نکرده است
    if(m_node == node)
        return;

    // حذف از Node قبلی
    if(m_node)
    {
        m_node->removePin(this);
    }

    // اتصال به Node جدید
    m_node = node;

    if(m_node)
    {
        m_node->addPin(this);
        m_node->setPosition(scenePos());

        if(m_node->net())
            m_node->net()->addPin(this);
    }

    updateWires();
}