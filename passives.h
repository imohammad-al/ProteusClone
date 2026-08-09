#ifndef PASSIVES_H
#define PASSIVES_H

#include "component.h"

// --- مقاومت پیشرفته ---
class Resistor : public Component { // بک‌اسلش اصلاح شد
public:
    Resistor(); // سازنده به فایل cpp منتقل شد

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override; // بدنه حذف شد تا با cpp تداخل نکند

    Component* clone() const override {
        return new Resistor();
    }

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }
};

// --- خازن پیشرفته ---
// حالت گذرا برای شبیه‌سازی آنالوگ (فاز ۱۳): ولتاژ دو سر خازن در تیک قبلی، برای
// مدل معادل نورتن روش Backward Euler در sim/analogsolver.h. فقط توسط AnalogSolver
// خوانده/نوشته می‌شود؛ در ابتدای هر Run (resetTransientState) از صفر (خازن
// کاملاً دشارژ - یک فرض شروع رایج و ساده) آغاز می‌شود.
class Capacitor : public Component {
public:
    Capacitor();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override; // بدنه حذف شد

    Component* clone() const override {
        return new Capacitor();
    }

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }

    double previousVoltage() const { return m_previousVoltage; }
    void setPreviousVoltage(double volts) { m_previousVoltage = volts; }
    void resetTransientState() { m_previousVoltage = 0.0; }

private:
    double m_previousVoltage = 0.0;
};

// --- سلف پیشرفته ---
// حالت گذرا مشابه خازن، اما به‌جای ولتاژ، جریانِ عبوری از سلف در تیک قبلی را
// نگه می‌دارد (طبق مدل معادل Backward Euler سلف در sim/analogsolver.h).
class Inductor : public Component {
public:
    Inductor();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override; // بدنه حذف شد

    Component* clone() const override {
        return new Inductor();
    }

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }

    double previousCurrent() const { return m_previousCurrent; }
    void setPreviousCurrent(double amperes) { m_previousCurrent = amperes; }
    void resetTransientState() { m_previousCurrent = 0.0; }

private:
    double m_previousCurrent = 0.0;
};

#endif // PASSIVES_H