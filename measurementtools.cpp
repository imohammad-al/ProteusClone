#include "measurementtools.h"
#include "pin.h"
#include "node.h"
#include <QPainter>
#include <algorithm>
#include <cmath>

// ============================= Logic Probe =============================
LogicProbe::LogicProbe() {
    setComponentType("Logic Probe");
    setCategory("Measurement");

    Pin* in = new Pin(this); in->setPos(0, 15); in->setDirection(PinDirection::Input); addPin(in);
}

QRectF LogicProbe::boundingRect() const { return QRectF(-15, -20, 30, 40); }
QPainterPath LogicProbe::shape() const { QPainterPath p; p.addRect(boundingRect()); return p; }

void LogicProbe::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    LogicValue value = LogicValue::Undefined;
    if (Pin *p = pin(0)) {
        if (Node *n = p->node())
            value = n->resolvedValue();
    }

    QColor bg = Qt::lightGray;
    QString label = "?";
    if (value == LogicValue::High) { bg = Qt::green; label = "H"; }
    else if (value == LogicValue::Low) { bg = Qt::darkGray; label = "L"; }

    painter->setPen(QPen(Qt::black, 2));
    painter->drawLine(0, 15, 0, 8);

    painter->setBrush(bg);
    painter->drawEllipse(QPointF(0, -3), 12, 12);
    painter->setBrush(Qt::NoBrush);

    painter->drawText(QRectF(-8, -11, 16, 16), Qt::AlignCenter, label);
}

Component* LogicProbe::clone() const { return new LogicProbe(); }

// ============================= Digital Voltmeter =============================
DigitalVoltmeter::DigitalVoltmeter() {
    setComponentType("Digital Voltmeter");
    setCategory("Measurement");
    setProperty("supplyVoltage", 5.0, "Supply Voltage", "V");

    Pin* in = new Pin(this); in->setPos(0, 20); in->setDirection(PinDirection::Input); addPin(in);
}

QRectF DigitalVoltmeter::boundingRect() const { return QRectF(-30, -35, 60, 65); }
QPainterPath DigitalVoltmeter::shape() const { QPainterPath p; p.addRect(boundingRect()); return p; }

void DigitalVoltmeter::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    LogicValue value = LogicValue::Undefined;
    if (Pin *p = pin(0)) {
        if (Node *n = p->node())
            value = n->resolvedValue();
    }

    painter->setPen(QPen(Qt::black, 2));
    painter->drawLine(0, 20, 0, 17);
    painter->drawEllipse(QPointF(0, -3), 22, 22);

    QString text = "? V";
    if (value == LogicValue::High)
        text = QString::number(property("supplyVoltage").toDouble(), 'g', 3) + " V";
    else if (value == LogicValue::Low)
        text = "0 V";

    painter->drawText(QRectF(-20, -13, 40, 20), Qt::AlignCenter, text);
    painter->drawText(-15, -30, name());
}

Component* DigitalVoltmeter::clone() const { return new DigitalVoltmeter(); }

// ============================= Analog Voltmeter =============================
AnalogVoltmeter::AnalogVoltmeter() {
    setComponentType("Analog Voltmeter");
    setCategory("Measurement");

    // هر دو پایه عمداً بدون setDirection رها شده‌اند (پیش‌فرض Bidirectional)
    // تا این قطعه مثل Resistor در سیستم دیجیتال هیچ نقشی نداشته باشد و در
    // AnalogSolver هم چیزی نراند - یک ولت‌متر ایده‌آل فقط می‌خواند.
    Pin *pPlus = new Pin(this);  pPlus->setPos(-25, 0); addPin(pPlus);
    Pin *pMinus = new Pin(this); pMinus->setPos(25, 0); addPin(pMinus);
}

QRectF AnalogVoltmeter::boundingRect() const { return QRectF(-30, -25, 60, 50); }
QPainterPath AnalogVoltmeter::shape() const { QPainterPath p; p.addRect(boundingRect()); return p; }

void AnalogVoltmeter::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    Node *nPlus = pin(0) ? pin(0)->node() : nullptr;
    Node *nMinus = pin(1) ? pin(1)->node() : nullptr;

    QString text = QStringLiteral("ERR");
    if (nPlus && nMinus && nPlus->hasVoltage() && nMinus->hasVoltage()) {
        const double diff = nPlus->voltage() - nMinus->voltage();
        text = QString::number(diff, 'g', 4) + " V";
    }

    painter->setPen(QPen(Qt::black, 2));
    painter->drawLine(-25, 0, -18, 0);
    painter->drawLine(18, 0, 25, 0);
    painter->drawEllipse(QPointF(0, 0), 18, 18);
    painter->drawText(QRectF(-18, -10, 36, 20), Qt::AlignCenter, text);
    painter->drawText(-15, -22, name());
}

Component* AnalogVoltmeter::clone() const { return new AnalogVoltmeter(); }

// ============================= Analog Ammeter =============================
AnalogAmmeter::AnalogAmmeter() {
    setComponentType("Analog Ammeter");
    setCategory("Measurement");

    // این دو پایه هم Bidirectional می‌مانند؛ AnalogSolver خودش موقع ساخت
    // دستگاه معادلات، این قطعه را مثل یک منبع ولتاژ ۰ ولت می‌شناسد و رفتار
    // می‌کند - نه از طریق جهت پایه.
    Pin *p1 = new Pin(this); p1->setPos(-25, 0); addPin(p1);
    Pin *p2 = new Pin(this); p2->setPos(25, 0);  addPin(p2);
}

QRectF AnalogAmmeter::boundingRect() const { return QRectF(-30, -25, 60, 50); }
QPainterPath AnalogAmmeter::shape() const { QPainterPath p; p.addRect(boundingRect()); return p; }

void AnalogAmmeter::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    QString text = QStringLiteral("--");
    if (m_hasMeasurement) {
        const double absVal = std::fabs(m_measuredCurrent);
        if (absVal >= 1.0)
            text = QString::number(m_measuredCurrent, 'g', 4) + " A";
        else if (absVal >= 1e-3)
            text = QString::number(m_measuredCurrent * 1e3, 'g', 4) + " mA";
        else
            text = QString::number(m_measuredCurrent * 1e6, 'g', 4) + QStringLiteral(" \u00B5A");
    }

    painter->setPen(QPen(Qt::black, 2));
    painter->drawLine(-25, 0, -18, 0);
    painter->drawLine(18, 0, 25, 0);
    painter->drawEllipse(QPointF(0, 0), 18, 18);
    painter->drawText(QRectF(-18, -10, 36, 20), Qt::AlignCenter, text);
    painter->drawText(-15, -22, name());
}

Component* AnalogAmmeter::clone() const { return new AnalogAmmeter(); }

void AnalogAmmeter::setMeasuredCurrent(double amperes) {
    m_measuredCurrent = amperes;
    m_hasMeasurement = true;
}

void AnalogAmmeter::clearMeasurement() {
    m_measuredCurrent = 0.0;
    m_hasMeasurement = false;
}


// ============================= Oscilloscope =============================
Oscilloscope::Oscilloscope() {
    setComponentType("Oscilloscope");
    setCategory("Measurement");

    Pin* ch1 = new Pin(this); ch1->setPos(-65, -15); ch1->setDirection(PinDirection::Input); addPin(ch1);
    Pin* ch2 = new Pin(this); ch2->setPos(-65, 15);  ch2->setDirection(PinDirection::Input); addPin(ch2);
}

QRectF Oscilloscope::boundingRect() const { return QRectF(-70, -45, 150, 90); }
QPainterPath Oscilloscope::shape() const { QPainterPath p; p.addRect(boundingRect()); return p; }

void Oscilloscope::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setPen(QPen(Qt::black, 2));
    painter->drawLine(-70, -15, -60, -15);
    painter->drawLine(-70, 15, -60, 15);

    const QRectF screenRect(-60, -35, 130, 65);
    painter->setBrush(QColor(10, 20, 10));
    painter->drawRect(screenRect);
    painter->setBrush(Qt::NoBrush);

    auto drawTrace = [&](const QVector<LogicValue> &history, const QColor &color, qreal baseY, qreal amplitude) {
        if (history.isEmpty())
            return;

        painter->setPen(QPen(color, 1.5));
        const qreal stepX = screenRect.width() / qMax(1, kMaxSamples);
        qreal x = screenRect.left();
        qreal prevY = baseY;

        for (int i = 0; i < history.size(); ++i) {
            const qreal y = (history[i] == LogicValue::High) ? (baseY - amplitude)
                          : (history[i] == LogicValue::Low)  ? (baseY + amplitude)
                                                              : baseY;
            painter->drawLine(QPointF(x, prevY), QPointF(x, y));   // پله عمودی (تغییر سطح)
            painter->drawLine(QPointF(x, y), QPointF(x + stepX, y)); // بخش افقی (نگه‌داشتن سطح)
            prevY = y;
            x += stepX;
        }
    };

    drawTrace(m_channel1History, QColor(255, 200, 0), screenRect.center().y() - 12, 10);
    drawTrace(m_channel2History, QColor(0, 220, 220), screenRect.center().y() + 12, 10);

    painter->setPen(Qt::black);
    painter->drawText(-25, -40, name().isEmpty() ? "Scope" : name());
}

Component* Oscilloscope::clone() const { return new Oscilloscope(); }

void Oscilloscope::sampleChannels()
{
    LogicValue v1 = LogicValue::Undefined;
    if (Pin *p = pin(0)) {
        if (Node *n = p->node())
            v1 = n->resolvedValue();
    }

    LogicValue v2 = LogicValue::Undefined;
    if (Pin *p = pin(1)) {
        if (Node *n = p->node())
            v2 = n->resolvedValue();
    }

    m_channel1History.append(v1);
    if (m_channel1History.size() > kMaxSamples)
        m_channel1History.removeFirst();

    m_channel2History.append(v2);
    if (m_channel2History.size() > kMaxSamples)
        m_channel2History.removeFirst();
}

void Oscilloscope::clearHistory()
{
    m_channel1History.clear();
    m_channel2History.clear();
}
