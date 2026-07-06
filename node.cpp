#include "node.h"
#include "junction.h"
#include "pin.h"
#include "wire.h"
Node::Node(QObject *parent)
    : QObject(parent)
{
}

//////////////////////////////////////////////////////////

void Node::addPin(Pin *pin)
{
    if(!pin)
        return;

    if(m_pins.contains(pin))
        return;

    m_pins.append(pin);

    m_position = pin->scenePos();

    if(m_junction)
    {
        m_junction->updatePosition();
        m_junction->updateVisibility();
    }
}

void Node::removePin(Pin *pin)
{
    m_pins.removeAll(pin);

    if(m_junction)
        m_junction->updateVisibility();
}
const QList<Pin*> &Node::pins() const
{
    return m_pins;
}

//////////////////////////////////////////////////////////

Junction *Node::junction() const
{
    return m_junction;
}

void Node::setJunction(Junction *junction)
{
    m_junction = junction;

    if(m_junction)
    {
        m_junction->updatePosition();
        m_junction->updateVisibility();
    }
}
//////////////////////////////////////////////////////////

QPointF Node::position() const
{
    return m_position;
}

void Node::setPosition(const QPointF &p)
{
    m_position = p;
}

//////////////////////////////////////////////////////////

QString Node::netName() const
{
    return m_netName;
}

void Node::setNetName(const QString &name)
{
    m_netName = name;
}

//////////////////////////////////////////////////////////

bool Node::highlighted() const
{
    return m_highlighted;
}

void Node::setHighlighted(bool value)
{
    m_highlighted = value;
}

void Node::highlight(bool on)
{
    setHighlighted(on);

    for(Wire *wire : m_wires)
    {
        if(wire)
            wire->setHighlighted(on);
    }
}

Net *Node::net() const
{
    return m_net;
}

void Node::setNet(Net *net)
{
    m_net = net;
}

const QList<Wire*> &Node::wires() const
{
    return m_wires;
}