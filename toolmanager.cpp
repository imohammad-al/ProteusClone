#include "toolmanager.h"

ToolManager::ToolManager(QObject *parent)
    : QObject(parent)
{
}

Tool ToolManager::currentTool() const
{
    return m_tool;
}

void ToolManager::setCurrentTool(Tool tool)
{
    if(m_tool == tool)
        return;

    m_tool = tool;

    emit toolChanged();
}

QString ToolManager::componentName() const
{
    return m_component;
}

void ToolManager::setComponentName(const QString &name)
{
    m_component = name;

    emit toolChanged();
}