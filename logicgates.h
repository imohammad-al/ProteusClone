#ifndef LOGICGATES_H
#define LOGICGATES_H

#include "digitalcomponent.h"

// --- گیت منطقی AND ---
class GateAND : public DigitalComponent {
public:
    GateAND();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;
    Component* clone() const override;

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }

    LogicValue computeOutput() const override;
};

// --- گیت منطقی OR ---
class GateOR : public DigitalComponent {
public:
    GateOR();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;
    Component* clone() const override;

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }

    LogicValue computeOutput() const override;
};

// --- گیت منطقی NOT ---
class GateNOT : public DigitalComponent {
public:
    GateNOT();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;
    Component* clone() const override;

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }

    LogicValue computeOutput() const override;
};

// --- گیت منطقی NAND ---
class GateNAND : public DigitalComponent {
public:
    GateNAND();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;
    Component* clone() const override;

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }

    LogicValue computeOutput() const override;
};

// --- گیت منطقی XOR ---
class GateXOR : public DigitalComponent {
public:
    GateXOR();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;
    Component* clone() const override;

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }

    LogicValue computeOutput() const override;
};

// --- فلیپ‌فلاپ نوع D (لبه بالارونده کلاک) ---
// پایه‌ها به ترتیب: D(0) - CLK(1) - Q(2) - Q̄(3)
// چون رفتار این قطعه فقط تابع ورودی فعلی نیست (به لبه کلاک و حافظه داخلی نیاز دارد)،
// به‌جای computeOutput()، خود simulationTick()/resetSimulation() بازنویسی شده‌اند.
class DFlipFlop : public DigitalComponent {
public:
    DFlipFlop();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;
    Component* clone() const override;

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }

    void simulationTick() override;
    void resetSimulation() override;

private:
    LogicValue m_lastClock = LogicValue::Undefined;
    LogicValue m_storedQ = LogicValue::Undefined;
};

#endif // LOGICGATES_H
