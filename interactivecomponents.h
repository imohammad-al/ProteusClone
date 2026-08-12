#ifndef INTERACTIVECOMPONENTS_H
#define INTERACTIVECOMPONENTS_H

#include "digitalcomponent.h"
#include "component.h"

// --- سوییچ دیجیتال ---
// یک منبع منطقی ساده با یک پایه خروجی. وضعیت آن (HIGH/LOW) از طریق ویژگی
// قابل‌ویرایش "state" در دیالوگ ویژگی‌ها (دابل‌کلیک روی قطعه) تغییر می‌کند.
// نکته: کلیک مستقیم روی بوم برای تغییر وضعیت حین اجرای شبیه‌سازی هنوز پیاده نشده
// و کاندید خوبی برای تکمیل در فاز بعدی رابط‌کاربری/تعامل است.
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

    // پایه دیجیتال خروجی همیشه اندیس ۰ است (نه آخرین پایه - پایه‌های ۱/۲ ترمینال‌های
    // آنالوگ‌اند). بدون این override، پیاده‌سازی پیش‌فرض DigitalComponent مقدار محاسبه‌شده
    // را روی پایه آخر (ترمینال آنالوگ B) می‌راند و پایه دیجیتال واقعی هیچ‌وقت مقدار
    // نمی‌گیرد - یک باگ واقعی جدا که هنگام بررسی LED پیدا شد (اگر LED به پایه دیجیتال
    // این قطعه وصل شود، هیچ‌وقت روشن نمی‌شد).
    int outputPinIndex() const override { return 0; }
};

// --- شاسی فشاری (Push Button) ---
// مشابه Switch اما از نظر مفهومی لحظه‌ای (Momentary) است؛ در این نسخه وضعیت آن هم
// از طریق ویژگی "pressed" کنترل می‌شود (تعامل مستقیم با کلیک ماوس، فاز بعدی).
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

    // همان دلیل Switch بالا - پایه دیجیتال خروجی پایه ۰ است، نه پایه آخر.
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
