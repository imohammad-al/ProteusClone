#ifndef COMPONENTFACTORY_H
#define COMPONENTFACTORY_H

#include <QString>
#include <QHash>
#include <functional>

class QGraphicsItem;

class ComponentFactory
{
public:

    using Creator = std::function<QGraphicsItem*()>;

    static void registerComponent(const QString &name,
                                  Creator creator);

    static QGraphicsItem *create(const QString &name);

private:

    static QHash<QString, Creator> m_creators;
};

#endif