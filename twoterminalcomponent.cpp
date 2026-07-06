#include "twoterminalcomponent.h"

TwoTerminalComponent::TwoTerminalComponent(
    QGraphicsItem *parent)
    : Component(parent)
{
    m_leftPin=new Pin(this);

    m_rightPin=new Pin(this);

    m_leftPin->setPos(-40,0);

    m_rightPin->setPos(40,0);

    addPin(m_leftPin);

    addPin(m_rightPin);
}

Pin *TwoTerminalComponent::leftPin() const
{
    return m_leftPin;
}

Pin *TwoTerminalComponent::rightPin() const
{
    return m_rightPin;
}