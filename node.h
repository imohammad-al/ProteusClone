#ifndef NODE_H
#define NODE_H

#include <QObject>
#include <QList>
#include <QPointF>
#include <QString>

class Pin;
class Wire;
class Junction;
class Net;
class Node : public QObject
{
    Q_OBJECT

public:
    explicit Node(QObject *parent = nullptr);

    //---------------------------------
    // Pins
    //---------------------------------

    void addPin(Pin *pin);
    void removePin(Pin *pin);

    const QList<Pin*> &pins() const;

    //---------------------------------
    // Junction
    //---------------------------------

    Junction *junction() const;
    void setJunction(Junction *j);

    //---------------------------------
    // Position
    //---------------------------------

    QPointF position() const;
    void setPosition(const QPointF &p);

    //---------------------------------
    // Net Name
    //---------------------------------

    QString netName() const;
    void setNetName(const QString &name);

    //---------------------------------
    // Highlight
    //---------------------------------

    bool highlighted() const;
    void setHighlighted(bool value);

    void highlight(bool on);

    Net *net() const;
    void setNet(Net *net);

    void addWire(Wire *wire);
    void removeWire(Wire *wire);

    const QList<Wire*> &wires() const;

private:

    QList<Pin*> m_pins;

    Junction *m_junction = nullptr;

    QPointF m_position;

    QString m_netName;

    QList<Wire*> m_wires;

    bool m_highlighted = false;

    Net *m_net = nullptr;
};

#endif