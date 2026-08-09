#ifndef JUNCTION_H
#define JUNCTION_H

#include <QGraphicsEllipseItem>

class Node;

class Junction : public QGraphicsEllipseItem
{
public:
    explicit Junction(Node *node,
                      QGraphicsItem *parent = nullptr);

    Node *node() const;

    void updatePosition();

    void updateVisibility();

private:

    Node *m_node = nullptr;
};

#endif