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
// محدودیت شناخته‌شده: هنوز فقط گرافیکی است، در AnalogSolver شرکت نمی‌کند. مدل
// Ebers-Moll واقعی (دو پیوند PN جفت‌شده + بهره جریان beta) به‌مراتب پیچیده‌تر
// از دیود تکی است (چند مجهول ولتاژ به‌جای یکی، دو معادله غیرخطی جفت‌شده) و
// عمداً به فاز جداگانه‌ای موکول شده - به HANDOFF.md مراجعه کن.
class TransistorNPN : public Component {
public:
    TransistorNPN(); // بدنه درون‌خطی حذف شد

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override; // بدنه درون‌خطی حذف شد
    Component* clone() const override;   // بدنه درون‌خطی حذف شد

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }
};

#endif // SEMICONDUCTORS_H