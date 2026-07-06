#ifndef RESISTOR_H
#define RESISTOR_H
#include "pin.h"
#include "twoterminalcomponent.h"
#include <QPainter>

class Resistor : public TwoTerminalComponent
{
    Q_OBJECT

public:
    explicit Resistor(QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *,
               QWidget *) override;
    Pin *leftPin() const;
    Pin *rightPin() const;



};

#endif