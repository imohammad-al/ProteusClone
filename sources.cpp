#include "sources.h"
#include "pin.h"
#include <QPainter>

// --- Ground ---
Ground::Ground() {
    setComponentType("Ground");
    setCategory("Sources");

    Pin* p1 = new Pin(this); p1->setPos(0, -20); addPin(p1); // فقط یک پین ورودی از بالا دارد

    // ساده‌سازی عمدی برای شبیه‌سازی دیجیتال: زمین همیشه سطح منطقی Low را می‌راند
    // (تا بتوان مدارات دیجیتال ساده مثل کلید+LED+زمین را بدون موتور آنالوگ کامل تست کرد)
    p1->setDirection(PinDirection::Output);
    p1->setDrivenValue(LogicValue::Low);
}
QRectF Ground::boundingRect() const { return QRectF(-20, -25, 40, 45); }
QPainterPath Ground::shape() const { QPainterPath p; p.addRect(QRectF(-15, -2, 30, 14)); return p; }
void Ground::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *) {
    painter->setPen(QPen(Qt::black, 2));
    painter->drawLine(0, -20, 0, 0);      // سیم اتصال
    painter->drawLine(-15, 0, 15, 0);     // خط اول زمین
    painter->drawLine(-10, 5, 10, 5);     // خط دوم
    painter->drawLine(-5, 10, 5, 10);     // خط سوم نوک تیز
    paintSelectionOverlay(painter, option);
}

// اضافه شد clone پیاده‌سازی متد
Component* Ground::clone() const {
    return new Ground();
}

// --- DC Voltage ---
DCVoltage::DCVoltage() {
    setComponentType("DC Source");
    setCategory("Sources");
    setProperty("voltage", 5.0, "Voltage", "V");

    Pin* p1 = new Pin(this); p1->setPos(-30, 0); addPin(p1);
    Pin* p2 = new Pin(this); p2->setPos(30, 0);  addPin(p2);

    // ساده‌سازی عمدی مشابه Ground: پایه + به‌عنوان منبع High و پایه - به‌عنوان منبع Low
    // در نظر گرفته می‌شود تا بشود این منبع را در مدارات دیجیتال هم به‌کار برد.
    p1->setDirection(PinDirection::Output);
    p1->setDrivenValue(LogicValue::High);
    p2->setDirection(PinDirection::Output);
    p2->setDrivenValue(LogicValue::Low);
}
QRectF DCVoltage::boundingRect() const { return QRectF(-35, -20, 70, 40); }
QPainterPath DCVoltage::shape() const { QPainterPath p; p.addEllipse(QPointF(0, 0), 15, 15); return p; }
void DCVoltage::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *) {
    painter->setPen(QPen(Qt::black, 2));
    painter->drawLine(-30, 0, -15, 0);
    painter->drawLine(15, 0, 30, 0);
    painter->drawEllipse(QPointF(0, 0), 15, 15); // دایره منبع ولتاژ
    painter->drawText(-10, 5, "+  -");
    painter->drawText(-10, -20, name() + " (" + property("voltage").toString() + "V)");
    paintSelectionOverlay(painter, option);
}

// اضافه شد clone پیاده‌سازی متد
Component* DCVoltage::clone() const {
    return new DCVoltage();
}

// --- Battery ---
Battery::Battery() {
    setComponentType("Battery");
    setCategory("Sources");
    setProperty("voltage", 9.0, "Voltage", "V");
    setProperty("internalResistance", 0.0, "Internal Resistance (not yet in AnalogSolver)", "Ω");

    Pin* p1 = new Pin(this); p1->setPos(-30, 0); addPin(p1);
    Pin* p2 = new Pin(this); p2->setPos(30, 0);  addPin(p2);

    // همون ساده‌سازی عمدی DCVoltage: پایه + منبع High، پایه - منبع Low، تا
    // بشه از باتری در مدارات کاملاً دیجیتالی هم استفاده کرد.
    p1->setDirection(PinDirection::Output);
    p1->setDrivenValue(LogicValue::High);
    p2->setDirection(PinDirection::Output);
    p2->setDrivenValue(LogicValue::Low);
}
QRectF Battery::boundingRect() const { return QRectF(-35, -20, 70, 40); }
QPainterPath Battery::shape() const { QPainterPath p; p.addRect(QRectF(-12, -12, 24, 24)); return p; }
void Battery::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *) {
    painter->setPen(QPen(Qt::black, 2));
    painter->drawLine(-30, 0, -12, 0);
    painter->drawLine(12, 0, 30, 0);
    // نماد استاندارد باتری: دو خط بلند/کوتاه متناوب
    painter->drawLine(-12, -12, -12, 12);
    painter->drawLine(-4, -6, -4, 6);
    painter->drawLine(4, -12, 4, 12);
    painter->drawLine(12, -6, 12, 6);
    painter->drawText(-10, -18, name() + " (" + property("voltage").toString() + "V)");
    paintSelectionOverlay(painter, option);
}
Component* Battery::clone() const { return new Battery(); }

// --- Clock Generator ---
ClockGenerator::ClockGenerator() {
    setComponentType("Clock Generator");
    setCategory("Sources");
    setProperty("periodTicks", 6.0, "Period", "ticks");

    Pin* out = new Pin(this); out->setPos(25, 0); out->setDirection(PinDirection::Output); addPin(out);
}

QRectF ClockGenerator::boundingRect() const { return QRectF(-25, -20, 55, 40); }
QPainterPath ClockGenerator::shape() const { QPainterPath p; p.addRect(QRectF(-12, -12, 27, 24)); return p; }

void ClockGenerator::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *) {
    painter->setPen(QPen(Qt::black, 2));
    painter->drawLine(-20, 0, -12, 0);
    painter->drawLine(15, 0, 25, 0);
    painter->drawRect(QRectF(-12, -12, 27, 24));

    // آیکون موج مربعی کوچک داخل بدنه به‌عنوان نماد کلاک
    QPainterPath wave;
    wave.moveTo(-8, 4);
    wave.lineTo(-8, -6);
    wave.lineTo(-1, -6);
    wave.lineTo(-1, 4);
    wave.lineTo(6, 4);
    wave.lineTo(6, -6);
    painter->drawPath(wave);

    painter->drawText(-15, -16, name());
    paintSelectionOverlay(painter, option);
}

Component* ClockGenerator::clone() const { return new ClockGenerator(); }

void ClockGenerator::simulationTick()
{
    const int periodTicks = qMax(1, int(property("periodTicks").toDouble()));
    m_ticksElapsed++;
    if (m_ticksElapsed >= periodTicks) {
        m_ticksElapsed = 0;
        m_state = (m_state == LogicValue::Low) ? LogicValue::High : LogicValue::Low;
    }

    if (Pin *out = pin(0))
        out->setDrivenValue(m_state);
}

void ClockGenerator::resetSimulation()
{
    m_state = LogicValue::Low;
    m_ticksElapsed = 0;

    if (Pin *out = pin(0))
        out->setDrivenValue(LogicValue::Undefined);
}