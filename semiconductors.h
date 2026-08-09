#ifndef SEMICONDUCTORS_H
#define SEMICONDUCTORS_H

#include "component.h"

// --- دیود ---
// از فاز ۱۴ به بعد، این قطعه در sim/analogsolver.h به‌صورت غیرخطی واقعی (معادله
// شاکلی + تکرار نیوتن-رافسون) حل می‌شود، نه فقط گرافیکی. ویژگی‌های عددی
// "saturationCurrent" (Is) و "idealityFactor" (n) پارامترهای معادله‌اند؛
// مقادیر پیش‌فرض تقریباً مطابق پارامترهای رایج SPICE برای 1N4148 است.
//
// m_previousVoltageGuess حالت گذرا برای Newton-Raphson است: نقطه شروع حدس هر
// تیک، آخرین ولتاژ همگراشده تیک قبل است (نه از صفر شروع کردن هر بار) - هم
// سریع‌تر همگرا می‌شود و هم از سرریز نمایی exp() با یک حدس اولیه بد جلوگیری
// می‌کند. فقط توسط AnalogSolver خوانده/نوشته می‌شود.
class Diode : public Component {
public:
    Diode(); // بدنه درون‌خطی حذف شد

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override; // بدنه درون‌خطی حذف شد
    Component* clone() const override;   // بدنه درون‌خطی حذف شد

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }

    double previousVoltageGuess() const { return m_previousVoltageGuess; }
    void setPreviousVoltageGuess(double volts) { m_previousVoltageGuess = volts; }
    void resetTransientState() { m_previousVoltageGuess = 0.0; }

private:
    double m_previousVoltageGuess = 0.0;
};

// --- ترانزیستور NPN ---
// از فاز ۱۶ به بعد، این قطعه در sim/analogsolver.cpp به‌صورت غیرخطی واقعی حل
// می‌شود: مدل کامل Ebers-Moll (دو پیوند PN جفت‌شده B-E و B-C + دو منبع جریان
// وابسته alphaF/alphaR) با همان زیرساخت نیوتن-رافسون فاز ۱۴ (دیود). پایه‌ها:
// pin(0)=Base، pin(1)=Collector، pin(2)=Emitter.
//
// ویژگی‌های عددی («saturationCurrent»، «idealityFactor» - مشابه دیود، این‌بار
// برای هر دو پیوند مشترک - ساده‌سازی عمدی) + «forwardBeta»/«reverseBeta» که
// AnalogSolver خودش به alphaF=βF/(βF+1) و alphaR=βR/(βR+1) تبدیل می‌کند.
//
// m_previousVBEGuess/m_previousVBCGuess حالت گذرا برای نیوتن-رافسون‌اند (نقطه
// شروع حدس هر تیک = آخرین جواب همگراشده‌ی تیک قبل) - دقیقاً همان الگوی
// Diode::m_previousVoltageGuess، فقط این‌بار دو مجهول به‌جای یکی چون ترانزیستور
// دو پیوند دارد. فقط توسط AnalogSolver خوانده/نوشته می‌شود.
class TransistorNPN : public Component {
public:
    TransistorNPN(); // بدنه درون‌خطی حذف شد

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override; // بدنه درون‌خطی حذف شد
    Component* clone() const override;   // بدنه درون‌خطی حذف شد

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }

    double previousVBEGuess() const { return m_previousVBEGuess; }
    double previousVBCGuess() const { return m_previousVBCGuess; }
    void setPreviousVBEGuess(double volts) { m_previousVBEGuess = volts; }
    void setPreviousVBCGuess(double volts) { m_previousVBCGuess = volts; }
    void resetTransientState() { m_previousVBEGuess = 0.0; m_previousVBCGuess = 0.0; }

private:
    double m_previousVBEGuess = 0.0;
    double m_previousVBCGuess = 0.0;
};

#endif // SEMICONDUCTORS_H