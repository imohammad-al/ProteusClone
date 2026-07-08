#include "componentfactory.h"
#include "capacitor.h"

QHash<QString,
      ComponentFactory::Creator>
    ComponentFactory::m_creators;

void ComponentFactory::registerComponent(
    const QString &name,
    Creator creator)
{
    m_creators[name] = creator;
}

QGraphicsItem *ComponentFactory::create(
    const QString &name)
{
    if(m_creators.contains(name))
        return m_creators[name]();

    return nullptr;
}