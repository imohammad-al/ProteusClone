#ifndef SOURCES_H
#define SOURCES_H

#include "component.h"
#include "digitalcomponent.h"

// --- زمین پیشرفته ---
class Ground : public Component {
public:
    Ground(); // کدهای قدیمی حذف و فقط اعلان باقی ماند

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override; // بدنه تابع حذف شد
    Component* clone() const override;   // بدنه تابع حذف شد

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }
};

// --- منبع ولتاژ دی‌سی پیشرفته ---
class DCVoltage : public Component {
public:
    DCVoltage(); // کدهای قدیمی حذف و فقط اعلان باقی ماند

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override; // بدنه تابع حذف شد
    Component* clone() const override;   // بدنه تابع حذف شد

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }
};

// --- باتری (بخش ۶.۱ مستند) ---
// از دید رفتار دیجیتالی و ثبت در AnalogSolver دقیقاً مثل DCVoltage عمل می‌کند
// (پایه + = High/ولتاژش، پایه - = Low/زمین نسبی). تنها تفاوتش ویژگی اختیاری
// "internalResistance" است که مستند بهش اشاره می‌کند - این ویژگی فعلاً فقط
// یک عدد ذخیره‌شده روی قطعه است و هنوز در AnalogSolver اعمال نمی‌شود (یعنی
// باتری فعلاً مثل یک منبع ولتاژ ایده‌آل حل می‌شود، نه با افت ولتاژ داخلی
// واقعی) - این یک ساده‌سازی صادقانه برای این فاز است، فاز جداگانه‌ای لازم
// دارد تا این مقاومت را واقعاً در معادلات MNA وارد کند.
class Battery : public Component {
public:
    Battery();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;
    Component* clone() const override;

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }
};

// --- مولد کلاک (برای مدارات دیجیتال/فلیپ‌فلاپ) ---
// هر propertyی به‌نام periodTicks تعداد گام‌های شبیه‌سازی بین هر تعویض HIGH/LOW را مشخص می‌کند.
// چون رفتار این قطعه خودمختار است (به هیچ ورودی‌ای وابسته نیست)، simulationTick() را
// کامل بازنویسی می‌کند به‌جای استفاده از computeOutput().
class ClockGenerator : public DigitalComponent {
public:
    ClockGenerator();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;
    Component* clone() const override;

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }

    void simulationTick() override;
    void resetSimulation() override;

private:
    LogicValue m_state = LogicValue::Low;
    int m_ticksElapsed = 0;
};

#endif // SOURCES_H