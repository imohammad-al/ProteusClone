#ifndef INTERACTIVECOMPONENTS_H
#define INTERACTIVECOMPONENTS_H

#include "digitalcomponent.h"
#include "component.h"

// --- سوییچ دیجیتال + آنالوگ ---
// پایه ۰ دقیقاً همان رفتار قبلی را دارد (خروجی دیجیتال High/Low بر پایه ویژگی
// "state") - برای سازگاری با مدارها/پروژه‌های ذخیره‌شده‌ی قبلی، این پایه و
// رفتارش دست‌نخورده می‌ماند و outputPinIndex() صراحتاً ۰ برمی‌گرداند (نه پیش‌فرض
// pinCount()-1، چون با اضافه شدن پایه‌های جدید آن پیش‌فرض دیگر درست نیست).
//
// پایه‌های ۱ و ۲ (فاز جدید): دو ترمینال «کلید فیزیکی واقعی» طبق بند ۶.۳ مستند -
// "باز=مقاومت بی‌نهایت، بسته=مقاومت صفر". این دو پایه در AnalogSolver مثل یک
// Resistor با مقاومت خیلی کوچک (بسته) یا کاملاً غایب (باز) دیده می‌شوند؛ خودشان
// Bidirectional هستند و در سیستم دیجیتال شرکت نمی‌کنند (دقیقاً مثل Resistor).
// همین یک ویژگی "state" هم پایه ۰ (دیجیتال) و هم پایه‌های ۱/۲ (آنالوگ) را کنترل
// می‌کند - از دید کاربر یک سوییچ، از دید موتور دو نمایندگی مستقل و بی‌تداخل.
class Switch : public DigitalComponent {
public:
    Switch();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;
    Component* clone() const override;

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }

    LogicValue computeOutput() const override;
    int outputPinIndex() const override { return 0; }
};

// --- شاسی فشاری (Push Button) + آنالوگ ---
// مشابه Switch: پایه ۰ دیجیتال (رفتار قبلی دست‌نخورده، ویژگی "pressed")، پایه‌های
// ۱/۲ ترمینال‌های آنالوگ (بسته فقط تا وقتی pressed=true، طبق ماهیت لحظه‌ای این قطعه).
class PushButton : public DigitalComponent {
public:
    PushButton();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;
    Component* clone() const override;

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }

    LogicValue computeOutput() const override;
    int outputPinIndex() const override { return 0; }
};

// --- LED ---
// یک نشانگر خروجی؛ خودش هیچ‌چیزی را محاسبه/رانه نمی‌کند، فقط مقدار گره متصل به
// پایه ورودی‌اش را در paint() می‌خواند و بر همان اساس روشن/خاموش رسم می‌شود؛
// به همین دلیل از Component مستقیم ارث‌بری می‌کند نه DigitalComponent (نیازی به
// چرخه تاخیر/محاسبه ندارد).
class LED : public Component {
public:
    LED();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;
    Component* clone() const override;

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }
};

#endif // INTERACTIVECOMPONENTS_H
