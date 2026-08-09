#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <QObject>
#include <QList>

class Component;
class Connection;

class Circuit : public QObject
{
    Q_OBJECT

public:
    explicit Circuit(QObject *parent = nullptr);

    void addComponent(Component *component);
    void removeComponent(Component *component);

    QList<Component*> components() const;

private:

    QList<Component*> m_components;
};

#endif