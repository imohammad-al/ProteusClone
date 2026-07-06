#ifndef TOOLMANAGER_H
#define TOOLMANAGER_H

#include <QObject>

enum class Tool
{
    Select,
    Wire,
    PlaceComponent
};

class ToolManager : public QObject
{
    Q_OBJECT

public:

    explicit ToolManager(QObject *parent = nullptr);

    Tool currentTool() const;

    void setCurrentTool(Tool tool);

    QString componentName() const;

    void setComponentName(const QString &name);

signals:

    void toolChanged();

private:

    Tool m_tool = Tool::Select;

    QString m_component;
};

#endif