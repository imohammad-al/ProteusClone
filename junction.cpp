#include "junction.h"
#include "node.h"

#include <QBrush>
#include <QPen>

Junction::Junction(Node *node,
                   QGraphicsItem *parent)
    : QGraphicsEllipseItem(parent),
    m_node(node)
{
    setRect(-3,-3,6,6);

    setBrush(Qt::black);

    setPen(Qt::NoPen);

    setZValue(100);

    setVisible(false);
}

Node *Junction::node() const
{
    return m_node;
}

void Junction::updatePosition()
{
    if(!m_node)
        return;

    setPos(m_node->position());
}

void Junction::updateVisibility()
{
    if(!m_node)
        return;

    int connections =
        m_node->pins().size()
        + m_node->wires().size();

    // فقط وقتی بیش از دو اتصال وجود دارد نمایش داده شود
    setVisible(connections > 2);
}