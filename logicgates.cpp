#include "logicgates.h"
#include "digitallogic.h"
#include "pin.h"
#include "sim/simulationlogger.h"
#include <QPainter>

// یادداشت مشترک برای همه گیت‌های دوورودی این فایل:
// پایه ۰ و ۱ = ورودی (Input)، پایه ۲ = خروجی (Output). چیدمان گرافیکی از GateAND اصلی الگو گرفته شده.

// ============================= AND =============================
GateAND::GateAND() {
    setComponentType("Gate AND");
    setCategory("Logic Gates");

    Pin* in1 = new Pin(this); in1->setPos(-35, -10); in1->setDirection(PinDirection::Input);  addPin(in1);
    Pin* in2 = new Pin(this); in2->setPos(-35, 10);  in2->setDirection(PinDirection::Input);  addPin(in2);
    Pin* out = new Pin(this); out->setPos(25, 0);    out->setDirection(PinDirection::Output); addPin(out);
}

QRectF GateAND::boundingRect() const { return QRectF(-40, -25, 70, 50); }
QPainterPath GateAND::shape() const { QPainterPath p; p.addRect(boundingRect()); return p; }

void GateAND::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setPen(QPen(Qt::black, 2));
    painter->drawLine(-35, -10, -15, -10);
    painter->drawLine(-35, 10, -15, 10);
    painter->drawLine(15, 0, 25, 0);

    QPainterPath path;
    path.moveTo(-15, -20);
    path.lineTo(-15, 20);
    path.lineTo(0, 20);
    path.arcTo(-15, -20, 30, 40, -90, 180);
    path.closeSubpath();

    painter->drawPath(path);
    painter->drawText(-10, -22, name());
}

Component* GateAND::clone() const { return new GateAND(); }

LogicValue GateAND::computeOutput() const
{
    return logicAnd(inputValue(0), inputValue(1));
}

// ============================= OR =============================
GateOR::GateOR() {
    setComponentType("Gate OR");
    setCategory("Logic Gates");

    Pin* in1 = new Pin(this); in1->setPos(-35, -10); in1->setDirection(PinDirection::Input);  addPin(in1);
    Pin* in2 = new Pin(this); in2->setPos(-35, 10);  in2->setDirection(PinDirection::Input);  addPin(in2);
    Pin* out = new Pin(this); out->setPos(25, 0);    out->setDirection(PinDirection::Output); addPin(out);
}

QRectF GateOR::boundingRect() const { return QRectF(-40, -25, 70, 50); }
QPainterPath GateOR::shape() const { QPainterPath p; p.addRect(boundingRect()); return p; }

void GateOR::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setPen(QPen(Qt::black, 2));
    painter->drawLine(-32, -10, -18, -10);
    painter->drawLine(-32, 10, -18, 10);
    painter->drawLine(15, 0, 25, 0);

    // بدنه منحنی OR: یک قوس ورودی + یک نوک خروجی
    QPainterPath path;
    path.moveTo(-20, -20);
    path.quadTo(0, -20, 15, 0);
    path.quadTo(0, 20, -20, 20);
    path.quadTo(-8, 0, -20, -20);
    path.closeSubpath();

    painter->drawPath(path);
    painter->drawText(-10, -22, name());
}

Component* GateOR::clone() const { return new GateOR(); }

LogicValue GateOR::computeOutput() const
{
    return logicOr(inputValue(0), inputValue(1));
}

// ============================= NOT =============================
GateNOT::GateNOT() {
    setComponentType("Gate NOT");
    setCategory("Logic Gates");

    Pin* in1 = new Pin(this); in1->setPos(-35, 0); in1->setDirection(PinDirection::Input);  addPin(in1);
    Pin* out = new Pin(this); out->setPos(25, 0);  out->setDirection(PinDirection::Output); addPin(out);
}

QRectF GateNOT::boundingRect() const { return QRectF(-40, -20, 70, 40); }
QPainterPath GateNOT::shape() const { QPainterPath p; p.addRect(boundingRect()); return p; }

void GateNOT::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setPen(QPen(Qt::black, 2));
    painter->drawLine(-35, 0, -15, 0);
    painter->drawLine(20, 0, 25, 0);

    QPainterPath path;
    path.moveTo(-15, -15);
    path.lineTo(-15, 15);
    path.lineTo(15, 0);
    path.closeSubpath();
    painter->drawPath(path);

    // حباب نشانگر NOT
    painter->drawEllipse(QPointF(18, 0), 3, 3);
    painter->drawText(-10, -17, name());
}

Component* GateNOT::clone() const { return new GateNOT(); }

LogicValue GateNOT::computeOutput() const
{
    return logicNot(inputValue(0));
}

// ============================= NAND =============================
GateNAND::GateNAND() {
    setComponentType("Gate NAND");
    setCategory("Logic Gates");

    Pin* in1 = new Pin(this); in1->setPos(-35, -10); in1->setDirection(PinDirection::Input);  addPin(in1);
    Pin* in2 = new Pin(this); in2->setPos(-35, 10);  in2->setDirection(PinDirection::Input);  addPin(in2);
    Pin* out = new Pin(this); out->setPos(28, 0);    out->setDirection(PinDirection::Output); addPin(out);
}

QRectF GateNAND::boundingRect() const { return QRectF(-40, -25, 75, 50); }
QPainterPath GateNAND::shape() const { QPainterPath p; p.addRect(boundingRect()); return p; }

void GateNAND::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setPen(QPen(Qt::black, 2));
    painter->drawLine(-35, -10, -15, -10);
    painter->drawLine(-35, 10, -15, 10);
    painter->drawLine(18, 0, 28, 0);

    QPainterPath path;
    path.moveTo(-15, -20);
    path.lineTo(-15, 20);
    path.lineTo(0, 20);
    path.arcTo(-15, -20, 30, 40, -90, 180);
    path.closeSubpath();

    painter->drawPath(path);
    painter->drawEllipse(QPointF(18, 0), 3, 3); // حباب نشانگر NOT روی خروجی AND
    painter->drawText(-10, -22, name());
}

Component* GateNAND::clone() const { return new GateNAND(); }

LogicValue GateNAND::computeOutput() const
{
    return logicNand(inputValue(0), inputValue(1));
}

// ============================= XOR =============================
GateXOR::GateXOR() {
    setComponentType("Gate XOR");
    setCategory("Logic Gates");

    Pin* in1 = new Pin(this); in1->setPos(-35, -10); in1->setDirection(PinDirection::Input);  addPin(in1);
    Pin* in2 = new Pin(this); in2->setPos(-35, 10);  in2->setDirection(PinDirection::Input);  addPin(in2);
    Pin* out = new Pin(this); out->setPos(25, 0);    out->setDirection(PinDirection::Output); addPin(out);
}

QRectF GateXOR::boundingRect() const { return QRectF(-40, -25, 70, 50); }
QPainterPath GateXOR::shape() const { QPainterPath p; p.addRect(boundingRect()); return p; }

void GateXOR::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setPen(QPen(Qt::black, 2));
    painter->drawLine(-30, -10, -18, -10);
    painter->drawLine(-30, 10, -18, 10);
    painter->drawLine(15, 0, 25, 0);

    QPainterPath path;
    path.moveTo(-20, -20);
    path.quadTo(0, -20, 15, 0);
    path.quadTo(0, 20, -20, 20);
    path.quadTo(-8, 0, -20, -20);
    path.closeSubpath();

    painter->drawPath(path);
    // خط اضافه سمت چپ که نشانه XOR است
    QPainterPath extraLine;
    extraLine.moveTo(-25, -20);
    extraLine.quadTo(-13, 0, -25, 20);
    painter->drawPath(extraLine);

    painter->drawText(-10, -22, name());
}

Component* GateXOR::clone() const { return new GateXOR(); }

LogicValue GateXOR::computeOutput() const
{
    return logicXor(inputValue(0), inputValue(1));
}

// ============================= D Flip-Flop =============================
DFlipFlop::DFlipFlop() {
    setComponentType("D Flip-Flop");
    setCategory("Logic Gates");
    setPropagationDelaySteps(0); // اعمال فوری روی همان گامی که لبه کلاک رخ می‌دهد

    Pin* d   = new Pin(this); d->setPos(-35, -15);   d->setDirection(PinDirection::Input);   addPin(d);   // 0: D
    Pin* clk = new Pin(this); clk->setPos(-35, 15);  clk->setDirection(PinDirection::Input);  addPin(clk); // 1: CLK
    Pin* q   = new Pin(this); q->setPos(35, -15);    q->setDirection(PinDirection::Output);  addPin(q);   // 2: Q
    Pin* qn  = new Pin(this); qn->setPos(35, 15);    qn->setDirection(PinDirection::Output);  addPin(qn);  // 3: Q̄
}

QRectF DFlipFlop::boundingRect() const { return QRectF(-40, -25, 80, 50); }
QPainterPath DFlipFlop::shape() const { QPainterPath p; p.addRect(boundingRect()); return p; }

void DFlipFlop::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setPen(QPen(Qt::black, 2));

    painter->drawLine(-35, -15, -20, -15);
    painter->drawLine(-35, 15, -20, 15);
    painter->drawLine(20, -15, 35, -15);
    painter->drawLine(20, 15, 35, 15);

    painter->drawRect(QRectF(-20, -20, 40, 40));

    painter->drawText(-16, -12, "D");
    painter->drawText(-16, 18, "CLK");
    painter->drawText(6, -12, "Q");
    painter->drawText(4, 18, "Q'");

    painter->drawText(-15, -25, name());
}

Component* DFlipFlop::clone() const { return new DFlipFlop(); }

void DFlipFlop::simulationTick()
{
    const LogicValue clk = inputValue(1); // CLK
    const LogicValue d = inputValue(0);   // D

    // پیام دقیق بند ۶.۴ مستند - وضعیت Undefined در D یا CLK باید هشدار تولید کند.
    if (d == LogicValue::Undefined || clk == LogicValue::Undefined) {
        SimulationLogger::instance().log(LogLevel::Warning,
            QStringLiteral("Floating input detected."));
    }

    // لبه بالارونده: فقط وقتی مقدار قبلی صراحتاً Low و مقدار فعلی صراحتاً High باشد
    if (m_lastClock == LogicValue::Low && clk == LogicValue::High) {
        m_storedQ = d;
    }
    m_lastClock = clk;

    if (Pin *q = pin(2))  q->setDrivenValue(m_storedQ);
    if (Pin *qn = pin(3)) qn->setDrivenValue(logicNot(m_storedQ));
}

void DFlipFlop::resetSimulation()
{
    m_lastClock = LogicValue::Undefined;
    m_storedQ = LogicValue::Undefined;

    if (Pin *q = pin(2))  q->setDrivenValue(LogicValue::Undefined);
    if (Pin *qn = pin(3)) qn->setDrivenValue(LogicValue::Undefined);
}
