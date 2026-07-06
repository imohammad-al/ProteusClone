#include "librarymanager.h"

#include "resistor.h"

QList<ComponentInfo> LibraryManager::m_components;

void LibraryManager::initialize()
{
    if(!m_components.isEmpty())
        return;

    ComponentInfo resistor;

    resistor.name = "Resistor";

    resistor.category = "Passive";

    resistor.description = "Electrical Resistor";

    resistor.creator = []()
    {
        return new Resistor;
    };

    m_components.append(resistor);
}

QList<ComponentInfo> LibraryManager::components()
{
    return m_components;
}