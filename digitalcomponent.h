#ifndef DIGITALCOMPONENT_H
#define DIGITALCOMPONENT_H

#include "component.h"
#include "pin.h"

// کلاس پایه مشترک همه قطعات دیجیتال (گیت‌های منطقی، فلیپ‌فلاپ، سوییچ، کلاک و...).
// مسئولیت این کلاس فقط "چرخه محاسبه‌ی یک گام شبیه‌سازی + تاخیر انتشار" است؛
// هیچ چیزی درباره گرافیک یا UI نمی‌داند (بخش ۶.۴ مستند: تاخیر انتشار و حالت Undefined).
//
// دو مدل استفاده:
//  ۱) قطعات ترکیبی ساده (گیت‌های AND/OR/NOT/...): فقط computeOutput() را بازنویسی می‌کنند؛
//     پیاده‌سازی پیش‌فرض simulationTick() نتیجه را با تاخیر propagationDelaySteps() اعمال می‌کند.
//  ۲) قطعات حالت‌دار/خودمختار (فلیپ‌فلاپ، کلاک): خود simulationTick() و resetSimulation()
//     را کامل بازنویسی می‌کنند چون رفتارشان صرفاً تابعی از ورودی فعلی نیست.
class DigitalComponent : public Component
{
public:
    explicit DigitalComponent(QGraphicsItem *parent = nullptr);

    // برای قطعات ترکیبی ساده: مقدار جدید خروجی را بر اساس ورودی‌های فعلی محاسبه می‌کند.
    virtual LogicValue computeOutput() const { return LogicValue::Undefined; }

    // اندیس پایه خروجی در لیست پایه‌های قطعه (پیش‌فرض: آخرین پایه اضافه‌شده)
    virtual int outputPinIndex() const { return pinCount() - 1; }

    int propagationDelaySteps() const { return m_delaySteps; }
    void setPropagationDelaySteps(int steps) { m_delaySteps = steps; }

    // توسط SimulationEngine در هر گام شبیه‌سازی فراخوانی می‌شود
    virtual void simulationTick();

    // بازنشانی به حالت اولیه شبیه‌سازی (خروجی Undefined، تاخیرهای معلق پاک می‌شوند)
    virtual void resetSimulation();

protected:
    // خواندن مقدار گره متصل به پایه ورودی شماره index (اگر پایه یا گره‌ای نباشد: Undefined)
    LogicValue inputValue(int index) const;

    // آیا حداقل یکی از پایه‌های *غیر از خروجی* این قطعه Undefined/Floating است؟
    // برای قطعات ترکیبی ساده (AND/OR/NOT/NAND/XOR/DFlipFlop) استفاده می‌شود تا پیام
    // دقیق «Floating input detected.» طبق بند ۶.۴ مستند صادر شود. قطعاتی که کاملاً
    // simulationTick خودشان را بازنویسی می‌کنند (MCU/LCD/Keypad/ADC/DAC) از این
    // تابع استفاده نمی‌کنند و منطق هشدار خودشان را دارند (در صورت نیاز).
    bool hasFloatingInput() const;

private:
    int m_delaySteps = 1;
    int m_pendingCountdown = -1;
    LogicValue m_pendingValue = LogicValue::Undefined;
};

#endif // DIGITALCOMPONENT_H
