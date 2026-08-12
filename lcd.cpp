#include "lcd.h"
#include "pin.h"
#include "sim/simulationlogger.h"

#include <QPainter>
#include <QFont>

LCD::LCD() {
    setComponentType("LCD 16x2");
    setCategory("Advanced");

    // پایه‌های D0-D7 (اندیس ۰..۷) + RS (اندیس ۸) + EN (اندیس ۹)، همه Input،
    // در یک ردیف عمودی سمت چپ قطعه (دقیقاً مثل چیدمان پورت‌های MCU در mcu.cpp).
    for (int i = 0; i < 10; ++i) {
        Pin *p = new Pin(this);
        p->setPos(-90, -45 + i * 10);
        p->setDirection(PinDirection::Input);
        addPin(p);
    }

    resetSimulation();
}

QRectF LCD::boundingRect() const { return QRectF(-95, -55, 210, 110); }
QPainterPath LCD::shape() const { QPainterPath p; p.addRect(boundingRect()); return p; }
Component* LCD::clone() const { return new LCD(); }

void LCD::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *) {
    // بدنه نمایشگر (سبز تیره، شبیه پس‌زمینه واقعی LCDهای کاراکتری)
    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(QColor(40, 70, 40));
    painter->drawRect(QRectF(-80, -45, 180, 90));
    painter->setBrush(Qt::NoBrush);

    // متن صفحه با فونت تک‌فاصله (هر سطر دقیقاً ۱۶ کاراکتر است)
    QFont mono;
    mono.setStyleHint(QFont::Monospace);
    mono.setFamily(QStringLiteral("Courier New"));
    mono.setPixelSize(13);
    painter->setFont(mono);
    painter->setPen(QColor(170, 255, 170));
    for (int r = 0; r < kRows; ++r) {
        painter->drawText(QRectF(-75, -30 + r * 24, 170, 20),
                           Qt::AlignLeft | Qt::AlignVCenter, m_screen[r]);
    }

    // برچسب پایه‌ها
    painter->setFont(QFont());
    painter->setPen(QPen(Qt::black, 1));
    static const char *const kPinLabels[10] =
        {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "RS", "EN"};
    for (int i = 0; i < 10; ++i) {
        const qreal y = -45 + i * 10;
        painter->drawLine(QPointF(-80, y), QPointF(-90, y));
        painter->drawText(QRectF(-95, y - 5, 14, 10), Qt::AlignRight | Qt::AlignVCenter, kPinLabels[i]);
    }

    painter->drawText(-78, -50, name());
    paintSelectionOverlay(painter, option);
}

void LCD::simulationTick()
{
    const LogicValue enable = inputValue(9); // پایه EN

    // فقط لبه بالارونده EN را در نظر بگیر - دقیقاً همان الگوی DFlipFlop::simulationTick
    if (m_lastEnable == LogicValue::Low && enable == LogicValue::High) {
        latchCurrentBus();
    }
    m_lastEnable = enable;
}

void LCD::latchCurrentBus()
{
    // ساخت بایت داده از پایه‌های D0-D7. پایه‌های Undefined به‌عنوان بیت صفر در
    // نظر گرفته می‌شوند - ساده‌سازی عمدی مشابه محدودیت مشابه در mcu.cpp (بدون
    // انتشار کامل حالت Undefined روی باس داده).
    quint8 dataByte = 0;
    for (int i = 0; i < 8; ++i) {
        if (inputValue(i) == LogicValue::High)
            dataByte |= static_cast<quint8>(1u << i);
    }

    const LogicValue rs = inputValue(8);
    if (rs == LogicValue::High) {
        writeCharacter(dataByte);
    } else if (rs == LogicValue::Low) {
        executeCommand(dataByte);
    }
    // اگر RS نامشخص (Floating/Undefined) باشد، این لبه EN عمداً نادیده گرفته می‌شود.
}

void LCD::executeCommand(quint8 commandByte)
{
    if (commandByte == 0x01) { // Clear Display
        for (int r = 0; r < kRows; ++r)
            m_screen[r] = QString(kColumns, QLatin1Char(' '));
        m_cursorRow = 0;
        m_cursorCol = 0;
    } else if (commandByte == 0x02) { // Return Home
        m_cursorRow = 0;
        m_cursorCol = 0;
    } else if (commandByte & 0x80) { // Set Cursor
        const int position = commandByte & 0x7F;
        m_cursorRow = (position >= kColumns) ? 1 : 0;
        m_cursorCol = position % kColumns;
    } else {
        SimulationLogger::instance().log(LogLevel::Warning,
            QObject::tr("دستور LCD ناشناخته نادیده گرفته شد: 0x%1")
                .arg(static_cast<int>(commandByte), 2, 16, QLatin1Char('0')));
    }
}

void LCD::writeCharacter(quint8 asciiCode)
{
    if (m_cursorRow >= 0 && m_cursorRow < kRows && m_cursorCol >= 0 && m_cursorCol < kColumns) {
        m_screen[m_cursorRow][m_cursorCol] = QLatin1Char(static_cast<char>(asciiCode));
    }
    advanceCursor();
}

void LCD::advanceCursor()
{
    ++m_cursorCol;
    if (m_cursorCol >= kColumns) {
        m_cursorCol = 0;
        m_cursorRow = (m_cursorRow + 1) % kRows;
    }
}

void LCD::resetSimulation()
{
    for (int r = 0; r < kRows; ++r)
        m_screen[r] = QString(kColumns, QLatin1Char(' '));
    m_cursorRow = 0;
    m_cursorCol = 0;
    m_lastEnable = LogicValue::Undefined;
}
