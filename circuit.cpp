#include "circuit.h"

#include "component.h"

Circuit::Circuit(QObject *parent)
    : QObject(parent)
{
}

void Circuit::addComponent(Component *component)
{
    if(!m_components.contains(component))
        m_components.append(component);
}

void Circuit::removeComponent(Component *component)
{
    m_components.removeAll(component);
}

QList<Component*> Circuit::components() const
{
    return m_components;
}