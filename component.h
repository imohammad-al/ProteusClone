#ifndef COMPONENT_H
#define COMPONENT_H

#include <QGraphicsObject>

class Component : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit Component(QGraphicsItem *parent = nullptr);
    int id() const;

    QString reference() const;
    QList<class Pin*> pins() const;

    void addPin(class Pin *pin);
    void updatePins();

    void updateConnections();


private:
    int m_id;
    QList<class Pin*> m_pins;
    static int s_nextId;
protected:
    QVariant itemChange(GraphicsItemChange change,
                        const QVariant &value) override;
};

#endif