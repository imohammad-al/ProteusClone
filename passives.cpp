#include "passives.h"
#include "pin.h"
#include <QPainter>

// --- Resistor ---
Resistor::Resistor() {
    setComponentType("Resistor");
    setCategory("Passive");
    setProperty("resistance", 1000.0, "Value", "Ω");

    Pin* p1 = new Pin(this); p1->setPos(-40, 0); addPin(p1);
    Pin* p2 = new Pin(this); p2->setPos(40, 0);  addPin(p2);
}
QRectF Resistor::boundingRect() const { return QRectF(-45, -15, 90, 30); }
QPainterPath Resistor::shape() const { QPainterPath p; p.addRect(boundingRect()); return p; }
void Resistor::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setPen(QPen(Qt::black, 2));
    painter->drawLine(-40, 0, -25, 0);
    painter->drawRect(-25, -10, 50, 20);
    painter->drawLine(25, 0, 40, 0);
    painter->drawText(-15, -15, name());
    painter->drawText(-15, 25, property("resistance").toString() + " Ω");
}

// --- Capacitor ---
Capacitor::Capacitor() {
    setComponentType("Capacitor");
    setCategory("Passive");
    setProperty("capacitance", 0.00001, "Value", "F");

    Pin* p1 = new Pin(this); p1->setPos(-30, 0); addPin(p1);
    Pin* p2 = new Pin(this); p2->setPos(30, 0);  addPin(p2);
}
QRectF Capacitor::boundingRect() const { return QRectF(-35, -20, 70, 40); }
QPainterPath Capacitor::shape() const { QPainterPath p; p.addRect(boundingRect()); return p; }
void Capacitor::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setPen(QPen(Qt::black, 2));
    painter->drawLine(-30, 0, -10, 0);
    painter->drawLine(10, 0, 30, 0);
    painter->drawLine(-10, -15, -10, 15);
    painter->drawLine(10, -15, 10, 15);
    painter->drawText(-10, -20, name());
}

// --- Inductor ---
Inductor::Inductor() {
    setComponentType("Inductor");
    setCategory("Passive");
    setProperty("inductance", 0.001, "Value", "H");

    Pin* p1 = new Pin(this); p1->setPos(-40, 0); addPin(p1);
    Pin* p2 = new Pin(this); p2->setPos(40, 0);  addPin(p2);
}
QRectF Inductor::boundingRect() const { return QRectF(-45, -15, 90, 30); }
QPainterPath Inductor::shape() const { QPainterPath p; p.addRect(boundingRect()); return p; }
void Inductor::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setPen(QPen(Qt::black, 2));
    painter->drawLine(-40, 0, -20, 0);
    // رسم یک موج ساده برای سلف
    for(int i=0; i<4; ++i) {
        painter->drawArc(-20 + (i*10), -10, 10, 20, 0, 180 * 16);
    }
    painter->drawLine(20, 0, 40, 0);
    painter->drawText(-10, -15, name());
}