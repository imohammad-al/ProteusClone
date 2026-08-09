#ifndef LCD_H
#define LCD_H

#include "digitalcomponent.h"
#include <array>

// نمایشگر کاراکتری ۱۶×۲ (بخش ۷.۸ مستند پروژه).
//
// نکته صادقانه مهم (مثل mcu.h): این یک تراشه واقعی HD44780 نیست؛ یک مدل
// ساده‌شده و کاملاً دیجیتال با همان مفاهیم اصلی است (باس داده ۸ بیتی + RS +
// Enable، دستور Clear/Set-Cursor، نوشتن کاراکتر با پیش‌روی خودکار مکان‌نما).
// فرمت دستورات دقیقاً پایین همین فایل مستند شده تا بشود از MCU (یا هر منبع
// دیجیتال دیگری) برایش دستور نوشت.
//
// پایه‌ها (همه Input - این قطعه فقط می‌خواند، چیزی را نمی‌راند):
//   0-7  : D0-D7   باس داده ۸ بیتی (کد دستور یا کد ASCII کاراکتر)
//   8    : RS      Register Select : Low=دستور ، High=داده(کاراکتر)
//   9    : EN      Enable/Strobe   : با لبه بالارونده مقدار فعلی D0-D7/RS لچ می‌شود
//
// فرمت دستورات (خوانده‌شده وقتی RS=Low موقع لبه بالارونده EN):
//   0x01        Clear Display   (پاک کردن کامل صفحه + بازگشت مکان‌نما به (0,0))
//   0x02        Return Home     (فقط بازگشت مکان‌نما به (0,0)، محتوای صفحه دست‌نخورده)
//   0x80..0x9F  Set Cursor      (۷ بیت پایین = موقعیت خطی ۰..۳۱؛ ۰-۱۵=ستون‌های سطر
//                                 بالا، ۱۶-۳۱=ستون‌های سطر پایین - مشابه ساده‌شده
//                                 دستور Set DDRAM Address واقعی HD44780)
//   سایر مقادیر: نادیده گرفته می‌شود و یک هشدار در پنجره گزارشات ثبت می‌شود.
//
// وقتی RS=High موقع لبه بالارونده EN: مقدار D0-D7 به‌عنوان کد ASCII یک کاراکتر
// در موقعیت فعلی مکان‌نما نوشته می‌شود و مکان‌نما یک خانه جلو می‌رود (با گردش
// خودکار به ابتدای سطر بعد، و از سطر دوم دوباره به اول سطر اول).
//
// محدودیت شناخته‌شده: فقط کدهای ASCII پایه (۰-۱۲۷) به‌طور قابل‌اعتماد پشتیبانی
// می‌شوند (دقیقاً مثل بخش پایه‌ی کاراکتر ROM واقعی HD44780).
class LCD : public DigitalComponent
{
public:
    LCD();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;
    Component* clone() const override;

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }

    // چون رفتار این قطعه فقط تابع ورودی فعلی نیست (به لبه EN و حافظه‌ی داخلی صفحه
    // نیاز دارد)، مثل DFlipFlop/MCU خودِ این دو تابع کامل بازنویسی شده‌اند.
    void simulationTick() override;
    void resetSimulation() override;

private:
    void latchCurrentBus();
    void executeCommand(quint8 commandByte);
    void writeCharacter(quint8 asciiCode);
    void advanceCursor();

    static constexpr int kColumns = 16;
    static constexpr int kRows = 2;

    std::array<QString, kRows> m_screen;
    int m_cursorRow = 0;
    int m_cursorCol = 0;

    LogicValue m_lastEnable = LogicValue::Undefined;
};

#endif // LCD_H
