#ifndef DAC_H
#define DAC_H

#include "digitalcomponent.h"

// مبدل دیجیتال به آنالوگ (DAC) - بخش ۷.۲ مستند پروژه.
//
// پایه‌ها:
//   0..7  : D0..D7   ورودی دیجیتال ۸ بیتی (Input)
//   8     : Vref+    (Bidirectional - آنالوگ)
//   9     : Vref-    (Bidirectional - آنالوگ)
//   10    : Vout+    (Bidirectional - آنالوگ؛ خروجی محاسبه‌شده)
//   11    : Vout-    (Bidirectional - آنالوگ؛ معمولاً به Ground وصل می‌شود)
//
// نگاشت خطی: کد ۰ → Vref- ، کد ۲۵۵ → Vref+ ، بین این دو خطی.
//
// نکته معماری مهم: sim/analogsolver.h این قطعه را - برخلاف DC Source/Battery که
// ولتاژشان از ویژگی "voltage" ثابت خوانده می‌شود - با یک dynamic_cast مستقیم
// می‌شناسد و outputVoltage() را صدا می‌زند، چون ولتاژ DAC هر تیک از روی ورودی
// دیجیتال محاسبه می‌شود، نه یک عدد ثابت.
//
// محدودیت زمان‌بندی صادقانه: چون در SimulationEngine::tick()، ابتدا AnalogSolver
// حل می‌شود و *سپس* DigitalComponentها (از جمله همین DAC) تیک می‌خورند، مقدار
// Vout که AnalogSolver در این تیک می‌بیند مربوط به خروجی *تیک قبلی* DAC است -
// یعنی همیشه حداقل یک تیک تاخیر ساختاری بین تغییر ورودی دیجیتال و اثرش روی
// شبکه آنالوگ وجود دارد، جدا از conversionDelayTicks (که رویش اضافه می‌شود،
// نه جایگزینش). این یک ساده‌سازی عمدی است، نه یک باگ فراموش‌شده.
class DAC : public DigitalComponent
{
public:
    DAC();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;
    Component* clone() const override;

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }

    void simulationTick() override;
    void resetSimulation() override;

    // فقط توسط sim/analogsolver.h خوانده می‌شود.
    double outputVoltage() const { return m_outputVoltage; }
    bool hasOutputVoltage() const { return m_hasOutputVoltage; }

private:
    void applyOutputVoltage(double volts);

    static constexpr int kBitCount = 8;
    static constexpr int kMaxCode = (1 << kBitCount) - 1;

    int m_lastCode = -1;
    int m_pendingCode = 0;
    int m_ticksRemaining = 0;
    double m_outputVoltage = 0.0;
    bool m_hasOutputVoltage = false;
};

#endif // DAC_H
