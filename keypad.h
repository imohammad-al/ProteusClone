#ifndef KEYPAD_H
#define KEYPAD_H

#include "digitalcomponent.h"

// صفحه‌کلید ماتریسی ۴×۴ (بخش ۷.۹ مستند پروژه).
//
// چیدمان استاندارد صفحه‌کلید:
//   1 2 3 A
//   4 5 6 B
//   7 8 9 C
//   * 0 # D
//
// پایه‌ها:
//   0-3 : Row0..Row3   (Output) - وضعیت سطرها؛ بر اساس اسکن ستون محاسبه می‌شود
//   4-7 : Col0..Col3   (Input)  - سیگنال اسکن ستون (باید از بیرون این قطعه رانده شود)
//
// منطق شبیه‌سازی: به‌جای کلیک مستقیم روی بوم (همان محدودیت شناخته‌شده‌ی
// Switch/PushButton - به interactivecomponents.h مراجعه کن)، کاربر کلید مورد
// نظرش را با تایپ نام آن در ویژگی «pressedKey» (دیالوگ ویژگی‌ها) انتخاب می‌کند.
// سپس هر گام شبیه‌سازی: برای هر سطر r، اگر ستونی که آن کلید در آن قرار دارد
// هم‌اکنون در حال اسکن (High) باشد، آن سطر High می‌شود؛ در غیر این صورت Low
// (یا Undefined اگر اصلاً هیچ ستونی درایو نشده - یعنی اسکنر متصل/فعال نیست).
// این دقیقاً همان منطق اسکن سطر/ستون واقعی است، فقط جهت خواندنش برعکس شده
// (این کلاس سطرها را می‌راند و ستون‌ها را می‌خواند).
//
// محدودیت شناخته‌شده: چون پورت‌های MCU در این نسخه فقط خروجی هستند (mcu.h را
// ببین)، MCU هنوز نمی‌تواند این سطرها را واقعاً بخواند تا یک اسکن کامل
// رفت‌وبرگشتی را خودش انجام دهد. اما منطق اسکن این قطعه کاملاً مستقل و قابل
// تست است (مثلاً با وصل چند Switch به پایه‌های ستون و مشاهده سطرها با Logic Probe).
class Keypad : public DigitalComponent
{
public:
    Keypad();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;
    Component* clone() const override;

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }

    // رفتار این قطعه فقط تابع ورودی فعلی نیست (خروجی چندگانه دارد، نه یک خروجی
    // مثل مدل computeOutput())، پس مثل DFlipFlop/MCU خودِ simulationTick بازنویسی شده.
    void simulationTick() override;
    void resetSimulation() override;

private:
    // موقعیت کلیدِ در حال حاضر انتخاب‌شده (ویژگی pressedKey) را در چیدمان ۴×۴ پیدا می‌کند
    bool findPressedPosition(int *row, int *col) const;

    static constexpr int kGridSize = 4;
};

#endif // KEYPAD_H
