#ifndef NODE_H
#define NODE_H

#include <QUuid>
#include <QList>
#include <QPointF>
#include "pin.h" // برای LogicValue (resolvedValue) و PinDirection

class Wire;
class Net;
class Junction;
class CircuitScene;

class Node {
public:
    Node(CircuitScene* scene = nullptr); // پذیرش ورودی پوینتر scene
    ~Node();

    QUuid id() const { return m_id; }

    void addPin(Pin* pin);
    void removePin(Pin* pin);
    QList<Pin*> pins() const { return m_pins; }

    void addWire(Wire* wire);
    void removeWire(Wire* wire);
    QList<Wire*> wires() const { return m_wires; }

    // متدهای سازگارکننده کدهای قدیمی
    Net* net() const { return m_net; }
    void setNet(Net* net) { m_net = net; }

    Junction* junction() const { return m_junction; }
    void setJunction(Junction* junction) { m_junction = junction; }
    // کد قبلی خط 35 در node.h را پاک کنید و این را بگذارید:
    QPointF pos() const;

    // مقدار منطقی این گره را از روی همه پایه‌های Output متصل به آن محاسبه می‌کند.
    // اگر هیچ پایه Output ای وجود نداشته باشد → Undefined (شناور/Floating).
    // اگر همه پایه‌های Output مقدار یکسانی بدهند → همان مقدار.
    // اگر دو پایه Output مقادیر متفاوت بدهند → Undefined + hasConflict=true (اتصال کوتاه منطقی).
    LogicValue resolvedValue(bool *hasConflict = nullptr) const;

    // ولتاژ آنالوگ این گره - کاملاً مستقل از resolvedValue() دیجیتال بالا و هیچ
    // تداخلی با آن ندارد (قطعات آنالوگ مثل Resistor پایه‌هایشان Bidirectional
    // هستند، پس اصلاً در resolvedValue() شرکت نمی‌کنند - به pin.h مراجعه کن).
    // این مقدار توسط sim/analogsolver.h محاسبه و اینجا فقط ذخیره می‌شود؛ Node
    // خودش هیچ محاسبه‌ای انجام نمی‌دهد. وقتی hasVoltage()==false باشد یعنی این
    // گره اصلاً جزو هیچ زیرمداری که تحلیل آنالوگ رویش انجام شده نبوده (مثلاً یک
    // مدار کاملاً دیجیتالی، یا بخشی از مدار که به زمین وصل نیست).
    bool hasVoltage() const { return m_hasVoltage; }
    double voltage() const { return m_voltage; }
    void setVoltage(double volts) { m_voltage = volts; m_hasVoltage = true; }
    void clearVoltage() { m_voltage = 0.0; m_hasVoltage = false; }

private:
    QUuid m_id;
    QList<Pin*> m_pins;
    QList<Wire*> m_wires;
    Net* m_net = nullptr;
    Junction* m_junction = nullptr;
    double m_voltage = 0.0;
    bool m_hasVoltage = false;
};

#endif // NODE_H