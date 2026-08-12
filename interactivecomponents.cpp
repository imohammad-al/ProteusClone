#include "interactivecomponents.h"
#include "pin.h"
#include "node.h"
#include <QPainter>

// ============================= Switch =============================
Switch::Switch() {
    setComponentType("Switch");
    setCategory("Interactive");
    setProperty("state", false, "State (ON=High)");

    Pin* out = new Pin(this); out->setPos(25, 0); out->setDirection(PinDirection::Output); addPin(out); // 0: دیجیتال (دست‌نخورده)
    Pin* a = new Pin(this); a->setPos(-20, 20); addPin(a); // 1: ترمینال آنالوگ A (Bidirectional پیش‌فرض)
    Pin* b = new Pin(this); b->setPos(25, 20);  addPin(b); // 2: ترمینال آنالوگ B
}

QRectF Switch::boundingRect() const { return QRectF(-25, -20, 55, 50); }
QPainterPath Switch::shape() const { QPainterPath p; p.addRect(QRectF(-15, -12, 35, 30)); return p; }

void Switch::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *) {
    const bool on = property("state").toBool();

    painter->setPen(QPen(Qt::black, 2));
    painter->drawLine(-20, 0, -10, 0);
    painter->drawLine(15, 0, 25, 0);

    painter->drawEllipse(QPointF(-10, 0), 3, 3);
    painter->drawEllipse(QPointF(15, 0), 3, 3);

    painter->setPen(QPen(on ? Qt::darkGreen : Qt::darkGray, 2));
    if (on)
        painter->drawLine(-8, 0, 13, 0);
    else
        painter->drawLine(-8, 0, 10, -12);

    // ترمینال‌های آنالوگ (پایه ۱/۲) - همان کلید، فقط دید مداری آنالوگ
    painter->setPen(QPen(Qt::blue, 1, Qt::DashLine));
    painter->drawLine(QPointF(-20, 8), QPointF(-20, 20));
    painter->drawLine(QPointF(25, 8), QPointF(25, 20));
    painter->drawText(QRectF(-30, 22, 60, 12), Qt::AlignCenter, QStringLiteral("A ⟷ B"));

    painter->setPen(QPen(Qt::black, 1));
    painter->drawText(-15, -22, name() + (on ? " [ON]" : " [OFF]"));
    paintSelectionOverlay(painter, option);
}

Component* Switch::clone() const { return new Switch(); }

LogicValue Switch::computeOutput() const
{
    return property("state").toBool() ? LogicValue::High : LogicValue::Low;
}

// ============================= PushButton =============================
PushButton::PushButton() {
    setComponentType("Push Button");
    setCategory("Interactive");
    setProperty("pressed", false, "Pressed (=High)");

    Pin* out = new Pin(this); out->setPos(25, 0); out->setDirection(PinDirection::Output); addPin(out); // 0: دیجیتال (دست‌نخورده)
    Pin* a = new Pin(this); a->setPos(-20, 20); addPin(a); // 1: ترمینال آنالوگ A
    Pin* b = new Pin(this); b->setPos(25, 20);  addPin(b); // 2: ترمینال آنالوگ B
}

QRectF PushButton::boundingRect() const { return QRectF(-25, -20, 55, 50); }
QPainterPath PushButton::shape() const { QPainterPath p; p.addRect(QRectF(-11, -13, 22, 26)); return p; }

void PushButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *) {
    const bool pressed = property("pressed").toBool();

    painter->setPen(QPen(Qt::black, 2));
    painter->drawLine(-20, 0, -8, 0);
    painter->drawLine(8, 0, 25, 0);

    painter->setBrush(pressed ? QBrush(Qt::darkGreen) : QBrush(Qt::lightGray));
    painter->drawRect(QRectF(-8, -10, 16, 20));
    painter->setBrush(Qt::NoBrush);

    // ترمینال‌های آنالوگ (پایه ۱/۲) - فقط تا وقتی pressed=true بسته‌اند
    painter->setPen(QPen(Qt::blue, 1, Qt::DashLine));
    painter->drawLine(QPointF(-20, 8), QPointF(-20, 20));
    painter->drawLine(QPointF(25, 8), QPointF(25, 20));
    painter->drawText(QRectF(-30, 22, 60, 12), Qt::AlignCenter, QStringLiteral("A ⟷ B"));

    painter->drawText(-15, -22, name() + (pressed ? " [PRESSED]" : ""));
    paintSelectionOverlay(painter, option);
}

Component* PushButton::clone() const { return new PushButton(); }

LogicValue PushButton::computeOutput() const
{
    return property("pressed").toBool() ? LogicValue::High : LogicValue::Low;
}

// ============================= LED =============================
LED::LED() {
    setComponentType("LED");
    setCategory("Interactive");

    Pin* in = new Pin(this); in->setPos(0, 20); in->setDirection(PinDirection::Input); addPin(in);
}

QRectF LED::boundingRect() const { return QRectF(-15, -25, 30, 50); }
// قلمرو برخورد LED دقیقاً همان دایره‌ی چراغ است (نه کل کادر شامل سیم پایه)
QPainterPath LED::shape() const { QPainterPath p; p.addEllipse(QPointF(0, -5), 12, 12); return p; }

void LED::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *) {
    LogicValue value = LogicValue::Undefined;
    if (Pin *p = pin(0)) {
        if (Node *n = p->node())
            value = n->resolvedValue();
    }

    QColor bodyColor = Qt::lightGray; // خاموش/نامشخص
    if (value == LogicValue::High) bodyColor = Qt::red;
    else if (value == LogicValue::Low) bodyColor = QColor(120, 40, 40); // قرمز کدر (خاموش قطعی)

    painter->setPen(QPen(Qt::black, 2));
    painter->drawLine(0, 20, 0, 8);

    painter->setBrush(bodyColor);
    painter->drawEllipse(QPointF(0, -5), 12, 12);
    painter->setBrush(Qt::NoBrush);

    painter->drawText(-10, -20, name());
    paintSelectionOverlay(painter, option);
}

Component* LED::clone() const { return new LED(); }
