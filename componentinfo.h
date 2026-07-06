#ifndef COMPONENTINFO_H
#define COMPONENTINFO_H

#include <QString>
#include <QGraphicsItem>
#include <functional>

struct ComponentInfo
{
    QString name;

    QString category;

    QString description;

    std::function<QGraphicsItem*()> creator;
};

#endif