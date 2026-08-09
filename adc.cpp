#include "adc.h"
#include "pin.h"
#include "node.h"
#include "sim/simulationlogger.h"

#include <QPainter>
#include <cmath>

ADC::ADC() {
    setComponentType("ADC");
    setCategory("Advanced");
    setProperty("conversionDelayTicks", 1.0, "Conversion Delay", "ticks");

    // پایه ۰: Vin (آنالوگ، Bidirectional پیش‌فرض - در دیجیتال شرکت نمی‌کند)
    Pin *vin = new Pin(this); vin->setPos(-45, -20); addPin(vin);
    // پایه ۱: Vref+
    Pin *vrefPlus = new Pin(this); vrefPlus->setPos(-45, 0); addPin(vrefPlus);
    // پایه ۲: Vref-
    Pin *vrefMinus = new Pin(this); vrefMinus->setPos(-45, 20); addPin(vrefMinus);

    // پایه‌های ۳..۱۰: D0..D7 خروجی دیجیتال
    for (int i = 0; i < kBitCount; ++i) {
        Pin *d = new Pin(this);
        d->setPos(45, -35 + i * 10);
        d->setDirection(PinDirection::Output);
        addPin(d);
    }

    resetSimulation();
}

QRectF ADC::boundingRect() const { return QRectF(-50, -40, 100, 80); }
QPainterPath ADC::shape() const { QPainterPath p; p.addRect(boundingRect()); return p; }
Component* ADC::clone() const { return new ADC(); }

void ADC::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(QColor(230, 230, 250));
    painter->drawRect(QRectF(-35, -35, 70, 70));
    painter->setBrush(Qt::NoBrush);

    painter->drawText(QRectF(-35, -12, 70, 24), Qt::AlignCenter, QStringLiteral("ADC\n%1bit").arg(kBitCount));
    painter->drawText(-30, -40, name());

    static const char *const kAnalogLabels[3] = {"Vin", "V+", "V-"};
    for (int i = 0; i < 3; ++i) {
        const qreal y = -20 + i * 20;
        painter->drawLine(QPointF(-45, y), QPointF(-35, y));
        painter->drawText(QRectF(-50, y - 6, 12, 12), Qt::AlignCenter, kAnalogLabels[i]);
    }
    for (int i = 0; i < kBitCount; ++i) {
        const qreal y = -35 + i * 10;
        painter->drawLine(QPointF(35, y), QPointF(45, y));
    }
}

void ADC::simulationTick()
{
    Node *nVin = pin(0) ? pin(0)->node() : nullptr;
    Node *nRefPlus = pin(1) ? pin(1)->node() : nullptr;
    Node *nRefMinus = pin(2) ? pin(2)->node() : nullptr;

    const bool allValid = nVin && nRefPlus && nRefMinus
                        && nVin->hasVoltage() && nRefPlus->hasVoltage() && nRefMinus->hasVoltage();
    if (!allValid) {
        SimulationLogger::instance().log(LogLevel::Warning,
            QObject::tr("ADC: ولتاژ Vin/Vref نامشخص است - آخرین خروجی معتبر حفظ می‌شود."));
        return; // خروجی قبلی دست‌نخورده می‌ماند، طبق مستند
    }

    const double vin = nVin->voltage();
    const double vrefPlus = nRefPlus->voltage();
    const double vrefMinus = nRefMinus->voltage();

    int idealCode;
    if (vrefPlus <= vrefMinus) {
        idealCode = 0; // بازه مرجع نامعتبر - حالت لبه‌ای محافظه‌کارانه
    } else if (vin <= vrefMinus) {
        idealCode = 0;
    } else if (vin >= vrefPlus) {
        idealCode = kMaxCode;
    } else {
        const double fraction = (vin - vrefMinus) / (vrefPlus - vrefMinus);
        idealCode = qBound(0, static_cast<int>(std::lround(fraction * kMaxCode)), kMaxCode);
    }

    const int delayTicks = qMax(0, static_cast<int>(property("conversionDelayTicks").toDouble()));

    if (idealCode != m_lastIdealCode) {
        m_pendingCode = idealCode;
        m_ticksRemaining = delayTicks;
        m_lastIdealCode = idealCode;
    }

    if (m_ticksRemaining > 0) {
        --m_ticksRemaining;
    } else if (m_appliedCode != m_pendingCode) {
        m_appliedCode = m_pendingCode;
        applyOutputCode(m_appliedCode);
    }
}

void ADC::applyOutputCode(int code)
{
    for (int i = 0; i < kBitCount; ++i) {
        if (Pin *p = pin(3 + i))
            p->setDrivenValue((code & (1 << i)) ? LogicValue::High : LogicValue::Low);
    }
}

void ADC::resetSimulation()
{
    m_lastIdealCode = -1;
    m_pendingCode = 0;
    m_appliedCode = -1;
    m_ticksRemaining = 0;
    for (int i = 0; i < kBitCount; ++i)
        if (Pin *p = pin(3 + i))
            p->setDrivenValue(LogicValue::Undefined);
}
