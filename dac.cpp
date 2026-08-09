#include "dac.h"
#include "pin.h"
#include "node.h"

#include <QPainter>

DAC::DAC() {
    setComponentType("DAC");
    setCategory("Advanced");
    setProperty("conversionDelayTicks", 1.0, "Conversion Delay", "ticks");

    // پایه‌های ۰..۷: D0..D7 ورودی دیجیتال
    for (int i = 0; i < kBitCount; ++i) {
        Pin *d = new Pin(this);
        d->setPos(-45, -35 + i * 10);
        d->setDirection(PinDirection::Input);
        addPin(d);
    }
    // پایه ۸: Vref+ ، پایه ۹: Vref- (هر دو Bidirectional پیش‌فرض - آنالوگ)
    Pin *vrefPlus = new Pin(this); vrefPlus->setPos(45, -15); addPin(vrefPlus);
    Pin *vrefMinus = new Pin(this); vrefMinus->setPos(45, 0); addPin(vrefMinus);
    // پایه ۱۰: Vout+ ، پایه ۱۱: Vout- (Bidirectional - AnalogSolver این‌ها را
    // با dynamic_cast مستقیم می‌شناسد، نه از طریق جهت پایه)
    Pin *voutPlus = new Pin(this); voutPlus->setPos(45, 15); addPin(voutPlus);
    Pin *voutMinus = new Pin(this); voutMinus->setPos(45, 30); addPin(voutMinus);

    resetSimulation();
}

QRectF DAC::boundingRect() const { return QRectF(-50, -40, 100, 80); }
QPainterPath DAC::shape() const { QPainterPath p; p.addRect(boundingRect()); return p; }
Component* DAC::clone() const { return new DAC(); }

void DAC::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(QColor(250, 235, 215));
    painter->drawRect(QRectF(-35, -35, 70, 70));
    painter->setBrush(Qt::NoBrush);

    painter->drawText(QRectF(-35, -12, 70, 24), Qt::AlignCenter, QStringLiteral("DAC\n%1bit").arg(kBitCount));
    painter->drawText(-30, -40, name());

    for (int i = 0; i < kBitCount; ++i) {
        const qreal y = -35 + i * 10;
        painter->drawLine(QPointF(-45, y), QPointF(-35, y));
    }
    static const char *const kAnalogLabels[4] = {"V+", "V-", "O+", "O-"};
    for (int i = 0; i < 4; ++i) {
        const qreal y = -15 + i * 15;
        painter->drawLine(QPointF(35, y), QPointF(45, y));
        painter->drawText(QRectF(38, y - 6, 12, 12), Qt::AlignCenter, kAnalogLabels[i]);
    }
}

void DAC::simulationTick()
{
    int code = 0;
    for (int i = 0; i < kBitCount; ++i) {
        if (inputValue(i) == LogicValue::High)
            code |= (1 << i);
        // بیت‌های Low/Undefined صفر در نظر گرفته می‌شوند - همان ساده‌سازی
        // عمدی مشابه lcd.cpp روی باس داده (بدون انتشار کامل Undefined).
    }

    const int delayTicks = qMax(0, static_cast<int>(property("conversionDelayTicks").toDouble()));
    if (code != m_lastCode) {
        m_pendingCode = code;
        m_ticksRemaining = delayTicks;
        m_lastCode = code;
    }

    if (m_ticksRemaining > 0) {
        --m_ticksRemaining;
        return; // هنوز داخل تاخیر تبدیل هستیم - خروجی قبلی دست‌نخورده می‌ماند
    }

    Node *nRefPlus = pin(8) ? pin(8)->node() : nullptr;
    Node *nRefMinus = pin(9) ? pin(9)->node() : nullptr;
    if (!nRefPlus || !nRefMinus || !nRefPlus->hasVoltage() || !nRefMinus->hasVoltage()) {
        m_hasOutputVoltage = false; // مرجع نامشخص - خروجی آنالوگ معتبر نیست
        return;
    }

    const double vrefPlus = nRefPlus->voltage();
    const double vrefMinus = nRefMinus->voltage();
    const double fraction = static_cast<double>(m_pendingCode) / static_cast<double>(kMaxCode);
    applyOutputVoltage(vrefMinus + fraction * (vrefPlus - vrefMinus));
}

void DAC::applyOutputVoltage(double volts)
{
    m_outputVoltage = volts;
    m_hasOutputVoltage = true;
}

void DAC::resetSimulation()
{
    m_lastCode = -1;
    m_pendingCode = 0;
    m_ticksRemaining = 0;
    m_outputVoltage = 0.0;
    m_hasOutputVoltage = false;
}
