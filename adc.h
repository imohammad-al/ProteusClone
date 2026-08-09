#ifndef ADC_H
#define ADC_H

#include "digitalcomponent.h"

// مبدل آنالوگ به دیجیتال (ADC) - بخش ۷.۱ مستند پروژه.
//
// پایه‌ها:
//   0 : Vin    (Bidirectional - آنالوگ؛ مقدارش از sim/analogsolver.h می‌آید)
//   1 : Vref+  (Bidirectional - آنالوگ)
//   2 : Vref-  (Bidirectional - آنالوگ)
//   3..10 : D0..D7  خروجی دیجیتال ۸ بیتی (Output - LogicValue)
//
// طبق مستند، ایده‌آل فرض می‌شود: نویز/آفست/بهره/غیرخطی‌بودن/Jitter مدل نمی‌شوند.
// نگاشت خطی: Vin<=Vref- → کد=۰ ، Vin>=Vref+ → کد=۲^۸-۱=۲۵۵ ، در غیر این صورت
// گرد کردن خطی بین این دو (اشباع در دو سر بازه، طبق بند ۷.۱).
//
// ویژگی "conversionDelayTicks": بعد از تغییر Vin، خروجی دیجیتال بلافاصله عوض
// نمی‌شود؛ بعد از این تعداد گام شبیه‌سازی اعمال می‌شود و در طول این مدت آخرین
// مقدار معتبر قبلی حفظ می‌شود (دقیقاً طبق متن مستند).
//
// محدودیت‌های شناخته‌شده (صادقانه):
//   - تعداد بیت خروجی در این نسخه ثابت (۸) و فقط در زمان کامپایل قابل تغییر
//     است، نه در زمان اجرا از UI - مثل عرض پورت‌های MCU که آن‌ها هم ثابتند.
//   - اگر Vin/Vref+/Vref- به هیچ زیرمداری که AnalogSolver حلش کرده وصل نباشند
//     (مثلاً هیچ Resistor/DC Source/Battery/Ground‌ای در مسیرشان نباشد)،
//     hasVoltage()==false می‌ماند؛ در این حالت ADC آخرین خروجی معتبر را حفظ
//     می‌کند و یک هشدار در پنجره گزارشات ثبت می‌کند - تبدیل نمی‌کند، خطا نمی‌دهد.
class ADC : public DigitalComponent
{
public:
    ADC();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;
    Component* clone() const override;

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }

    void simulationTick() override;
    void resetSimulation() override;

private:
    void applyOutputCode(int code);

    static constexpr int kBitCount = 8;
    static constexpr int kMaxCode = (1 << kBitCount) - 1;

    int m_lastIdealCode = -1;   // آخرین کد ایده‌آل محاسبه‌شده (برای تشخیص تغییر ورودی)
    int m_pendingCode = 0;      // کدی که پس از اتمام تاخیر اعمال خواهد شد
    int m_appliedCode = -1;     // آخرین کدی که واقعاً روی پایه‌ها اعمال شده
    int m_ticksRemaining = 0;   // شمارش معکوس تاخیر تبدیل باقی‌مانده
};

#endif // ADC_H
