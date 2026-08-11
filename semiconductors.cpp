#include "semiconductors.h"
#include "pin.h"
#include <QPainter>

// --- Diode ---
Diode::Diode() {
    setComponentType("Diode");
    setCategory("Semiconductors");
    setProperty("model", "1N4148"); // به ساختار پراپرتی کیوت منتقل شد
    // پارامترهای عددی معادله شاکلی برای تحلیل غیرخطی (فاز ۱۴) - تقریباً مطابق
    // پارامترهای رایج SPICE برای 1N4148.
    setProperty("saturationCurrent", 4.352e-9, "Saturation Current (Is)", "A");
    setProperty("idealityFactor", 1.906, "Ideality Factor (n)", "");

    Pin* p1 = new Pin(this); p1->setPos(-30, 0); addPin(p1); // آند
    Pin* p2 = new Pin(this); p2->setPos(30, 0);  addPin(p2); // کاتد
}

QRectF Diode::boundingRect() const { return QRectF(-35, -20, 70, 40); }
QPainterPath Diode::shape() const { QPainterPath p; p.addRect(boundingRect()); return p; }

void Diode::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setPen(QPen(Qt::black, 2));
    painter->drawLine(-30, 0, -10, 0);
    painter->drawLine(10, 0, 30, 0);

    // رسم مثلث دیود
    QPolygonF triangle;
    triangle << QPointF(-10, -12) << QPointF(10, 0) << QPointF(-10, 12);
    painter->drawPolygon(triangle);
    painter->drawLine(10, -12, 10, 12); // خط کاتد
    painter->drawText(-10, -18, name());
}

// پیاده‌سازی متد کلون برای دیود
Component* Diode::clone() const {
    return new Diode();
}

// --- Transistor NPN ---
TransistorNPN::TransistorNPN() {
    setComponentType("Transistor NPN");
    setCategory("Semiconductors");
    setProperty("model", "BC547"); // به ساختار پراپرتی کیوت منتقل شد

    Pin* base = new Pin(this);      base->setPos(-25, 0);   addPin(base);      // پایه بیس
    Pin* collector = new Pin(this); collector->setPos(15, -30); addPin(collector); // پایه کلکتور (بالا)
    Pin* emitter = new Pin(this);   emitter->setPos(15, 30);   addPin(emitter);   // پایه امیتر (پایین)
}

QRectF TransistorNPN::boundingRect() const { return QRectF(-30, -35, 55, 70); }
QPainterPath TransistorNPN::shape() const { QPainterPath p; p.addRect(boundingRect()); return p; }

void TransistorNPN::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setPen(QPen(Qt::black, 2));

    // رسم پایه‌ها و ساختار داخلی ترانزیستور
    painter->drawLine(-25, 0, -5, 0);     // سیم بیس
    painter->drawLine(-5, -15, -5, 15);   // خط اصلی بیس

    painter->drawLine(-5, -5, 15, -30);   // خط کلکتور داخلی به پین بالا
    painter->drawLine(-5, 5, 15, 30);     // خط امیتر داخلی به پین پایین

    // رسم فلش روی امیتر برای مشخص کردن NPN بودن
    QPolygonF arrow;
    arrow << QPointF(10, 23) << QPointF(15, 30) << QPointF(6, 28);
    painter->drawPolygon(arrow);

    painter->drawText(-25, -20, name());
}

// پیاده‌سازی متد کلون برای ترانزیستور
Component* TransistorNPN::clone() const {
    return new TransistorNPN();
}