#include "net.h"
#include "pin.h"
#include "wire.h"

Net::Net(QObject *parent)
    : QObject(parent)
{
}

void Net::addPin(Pin *pin)
{
    if(pin && !m_pins.contains(pin))
        m_pins.append(pin);
}

void Net::removePin(Pin *pin)
{
    m_pins.removeAll(pin);
}

void Net::addWire(Wire *wire)
{
    if(wire && !m_wires.contains(wire))
        m_wires.append(wire);
}

void Net::removeWire(Wire *wire)
{
    m_wires.removeAll(wire);
}

const QList<Pin*>& Net::pins() const
{
    return m_pins;
}

const QList<Wire*>& Net::wires() const
{
    return m_wires;
}

QString Net::name() const
{
    return m_name;
}

void Net::setName(const QString &name)
{
    m_name = name;
}