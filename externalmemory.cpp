#include "externalmemory.h"
#include "pin.h"

#include <QPainter>

ExternalMemory::ExternalMemory() {
    setComponentType("EEPROM/RAM");
    setCategory("Advanced");

    // پایه‌های آدرس A0-A7 (اندیس ۰..۷) - سمت چپ
    for (int i = 0; i < 8; ++i) {
        Pin *a = new Pin(this);
        a->setPos(-55, -35 + i * 10);
        a->setDirection(PinDirection::Input);
        addPin(a);
    }
    // پایه‌های داده D0-D7 (اندیس ۸..۱۵) - سمت راست، جهتشون پویا تنظیم می‌شود
    for (int i = 0; i < 8; ++i) {
        Pin *d = new Pin(this);
        d->setPos(55, -35 + i * 10);
        d->setDirection(PinDirection::Input);
        addPin(d);
    }
    // پایه WE (اندیس ۱۶) - پایین
    Pin *we = new Pin(this);
    we->setPos(0, 45);
    we->setDirection(PinDirection::Input);
    addPin(we);

    resetSimulation();
}

QRectF ExternalMemory::boundingRect() const { return QRectF(-60, -45, 120, 100); }
QPainterPath ExternalMemory::shape() const { QPainterPath p; p.addRect(boundingRect()); return p; }
Component* ExternalMemory::clone() const { return new ExternalMemory(); }

void ExternalMemory::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(QColor(220, 220, 220));
    painter->drawRect(QRectF(-45, -40, 90, 80));
    painter->setBrush(Qt::NoBrush);

    painter->drawText(QRectF(-45, -10, 90, 20), Qt::AlignCenter, QStringLiteral("EEPROM\n/RAM"));
    painter->drawText(-40, -45, name());

    for (int i = 0; i < 8; ++i) {
        const qreal y = -35 + i * 10;
        painter->drawLine(QPointF(-55, y), QPointF(-45, y));
    }
    for (int i = 0; i < 8; ++i) {
        const qreal y = -35 + i * 10;
        painter->drawLine(QPointF(45, y), QPointF(55, y));
    }
    painter->drawLine(QPointF(0, 40), QPointF(0, 45));
    painter->drawText(QRectF(-10, 28, 20, 12), Qt::AlignCenter, QStringLiteral("WE"));
}

void ExternalMemory::simulationTick()
{
    quint8 address = 0;
    for (int i = 0; i < 8; ++i)
        if (inputValue(i) == LogicValue::High)
            address |= static_cast<quint8>(1u << i);

    const LogicValue we = inputValue(16);

    if (we == LogicValue::High) {
        // حالت نوشتن: پایه‌های داده Input
        for (int i = 0; i < 8; ++i)
            if (Pin *p = pin(8 + i))
                p->setDirection(PinDirection::Input);

        // فقط روی لبه بالارونده WE بایت را لچ کن (دقیقاً الگوی LCD::simulationTick)
        if (m_lastWriteEnable == LogicValue::Low) {
            quint8 dataByte = 0;
            for (int i = 0; i < 8; ++i)
                if (inputValue(8 + i) == LogicValue::High)
                    dataByte |= static_cast<quint8>(1u << i);
            m_memory[address] = dataByte;
        }
    } else {
        // حالت خواندن: پایه‌های داده Output، پیوسته بایت آدرس فعلی را می‌رانند
        const quint8 value = m_memory[address];
        for (int i = 0; i < 8; ++i) {
            if (Pin *p = pin(8 + i)) {
                p->setDirection(PinDirection::Output);
                p->setDrivenValue((value & (1u << i)) ? LogicValue::High : LogicValue::Low);
            }
        }
    }

    m_lastWriteEnable = we;
}

void ExternalMemory::resetSimulation()
{
    // محتوای حافظه عمداً پاک نمی‌شود (طرف EEPROM را گرفتیم - به semiconductors.h
    // بالای همین فایل هدر مراجعه کن)؛ فقط وضعیت شبیه‌سازی بازنشانی می‌شود.
    m_lastWriteEnable = LogicValue::Undefined;
    for (int i = 0; i < 8; ++i) {
        if (Pin *p = pin(8 + i)) {
            p->setDirection(PinDirection::Input);
            p->setDrivenValue(LogicValue::Undefined);
        }
    }
}
