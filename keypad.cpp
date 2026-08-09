#include "keypad.h"
#include "pin.h"

#include <QPainter>

namespace {
// چیدمان استاندارد صفحه‌کلید ماتریسی (شرح کامل در keypad.h)
const char *const kLayout[4][4] = {
    {"1", "2", "3", "A"},
    {"4", "5", "6", "B"},
    {"7", "8", "9", "C"},
    {"*", "0", "#", "D"},
};
}

Keypad::Keypad() {
    setComponentType("Keypad 4x4");
    setCategory("Advanced");
    setProperty("pressedKey", QString("None"),
                "Pressed Key (1-9, A-D, *, #, or None)");

    // پایه‌های سطر (Output) - سمت چپ
    for (int r = 0; r < kGridSize; ++r) {
        Pin *p = new Pin(this);
        p->setPos(-55, -20 + r * 15);
        p->setDirection(PinDirection::Output);
        addPin(p);
    }
    // پایه‌های ستون (Input) - پایین
    for (int c = 0; c < kGridSize; ++c) {
        Pin *p = new Pin(this);
        p->setPos(-30 + c * 20, 55);
        p->setDirection(PinDirection::Input);
        addPin(p);
    }

    resetSimulation();
}

QRectF Keypad::boundingRect() const { return QRectF(-60, -45, 130, 115); }
QPainterPath Keypad::shape() const { QPainterPath p; p.addRect(boundingRect()); return p; }
Component* Keypad::clone() const { return new Keypad(); }

bool Keypad::findPressedPosition(int *row, int *col) const
{
    const QString pressed = property("pressedKey").toString();
    for (int r = 0; r < kGridSize; ++r) {
        for (int c = 0; c < kGridSize; ++c) {
            if (pressed == kLayout[r][c]) {
                if (row) *row = r;
                if (col) *col = c;
                return true;
            }
        }
    }
    return false;
}

void Keypad::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    int pressedRow = -1, pressedCol = -1;
    const bool hasPressed = findPressedPosition(&pressedRow, &pressedCol);

    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(QColor(230, 230, 230));
    painter->drawRect(QRectF(-45, -35, 90, 80));

    // شبکه ۴×۴ کلید، هرکدام با برچسب خودش؛ کلید انتخاب‌شده در ویژگی pressedKey هایلایت می‌شود
    for (int r = 0; r < kGridSize; ++r) {
        for (int c = 0; c < kGridSize; ++c) {
            const QRectF keyRect(-38 + c * 20, -28 + r * 18, 16, 16);
            const bool isPressed = hasPressed && r == pressedRow && c == pressedCol;
            painter->setBrush(isPressed ? QColor(120, 200, 120) : QColor(250, 250, 250));
            painter->drawRect(keyRect);
            painter->drawText(keyRect, Qt::AlignCenter, kLayout[r][c]);
        }
    }
    painter->setBrush(Qt::NoBrush);

    // برچسب پایه‌های سطر (چپ) و ستون (پایین)
    painter->setPen(QPen(Qt::black, 1));
    for (int r = 0; r < kGridSize; ++r) {
        const qreal y = -20 + r * 15;
        painter->drawLine(QPointF(-45, y), QPointF(-55, y));
        painter->drawText(QRectF(-60, y - 6, 12, 12), Qt::AlignCenter, QString("R%1").arg(r));
    }
    for (int c = 0; c < kGridSize; ++c) {
        const qreal x = -30 + c * 20;
        painter->drawLine(QPointF(x, 45), QPointF(x, 55));
        painter->drawText(QRectF(x - 6, 56, 12, 12), Qt::AlignCenter, QString("C%1").arg(c));
    }

    painter->drawText(-40, -40, name());
}

void Keypad::simulationTick()
{
    int pressedRow = -1, pressedCol = -1;
    const bool hasPressed = findPressedPosition(&pressedRow, &pressedCol);

    // خواندن مقدار فعلی چهار پایه ستون (اندیس ۴..۷)
    LogicValue colValue[kGridSize];
    bool anyColumnDriven = false;
    for (int c = 0; c < kGridSize; ++c) {
        colValue[c] = inputValue(kGridSize + c);
        if (colValue[c] != LogicValue::Undefined)
            anyColumnDriven = true;
    }

    // برای هر سطر: اگر ستونِ کلید فعلاً فشرده‌شده هم‌اکنون High اسکن شده باشد، آن
    // سطر High می‌شود؛ در غیر این صورت Low - مگر اینکه هیچ ستونی اصلاً درایو
    // نشده باشد (یعنی چیزی به این قطعه وصل نیست) که آنگاه Undefined می‌ماند.
    for (int r = 0; r < kGridSize; ++r) {
        LogicValue rowValue;
        if (!anyColumnDriven) {
            rowValue = LogicValue::Undefined;
        } else {
            const bool thisRowActive = hasPressed && r == pressedRow
                                        && colValue[pressedCol] == LogicValue::High;
            rowValue = thisRowActive ? LogicValue::High : LogicValue::Low;
        }
        if (Pin *rowPin = pin(r))
            rowPin->setDrivenValue(rowValue);
    }
}

void Keypad::resetSimulation()
{
    for (int r = 0; r < kGridSize; ++r) {
        if (Pin *rowPin = pin(r))
            rowPin->setDrivenValue(LogicValue::Undefined);
    }
}
