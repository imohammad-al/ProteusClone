#include "resistor.h"

Resistor::Resistor(QGraphicsItem *parent)
    : TwoTerminalComponent(parent)
{

}

QRectF Resistor::boundingRect() const
{
    return QRectF(-45,-20,90,40);
}

void Resistor::paint(QPainter *painter,
                     const QStyleOptionGraphicsItem *,
                     QWidget *)

{
    painter->setRenderHint(QPainter::Antialiasing, true);

    painter->setPen(QPen(Qt::black,2));

    painter->setBrush(Qt::NoBrush);

    painter->drawLine(-40,0,-25,0);

    painter->drawRect(-25,-10,50,20);

    painter->drawLine(25,0,40,0);


    painter->drawText(QPointF(-15,-15), reference());
}


