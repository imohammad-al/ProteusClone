#ifndef PIN_H
#define PIN_H

#include <QGraphicsObject>

class Wire;
class Node;

class Pin : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit Pin(QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *,
               QWidget *) override;

    void addWire(Wire *wire);

    Node *node() const;
    void setNode(Node *node);
    void updateWires();


private:
    QList<Wire*> m_wires;
    Node *m_node = nullptr;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    QVariant itemChange(GraphicsItemChange change,
                        const QVariant &value) override;
};

#endif