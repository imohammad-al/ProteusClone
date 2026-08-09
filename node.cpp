#include "node.h"
#include "pin.h"
#include "wire.h"

Node::Node(CircuitScene* scene) {
    Q_UNUSED(scene);
    m_id = QUuid::createUuid();
}

Node::~Node() {
    // گره فقط به اشیاء اشاره می‌کند و مالکیت حذف گرافیکی آن‌ها را ندارد
}

void Node::addPin(Pin* pin) {
    if (pin && !m_pins.contains(pin)) {
        m_pins.append(pin);
    }
}

void Node::removePin(Pin* pin) {
    if (pin) {
        m_pins.removeOne(pin);
    }
}

void Node::addWire(Wire* wire) {
    if (wire && !m_wires.contains(wire)) {
        m_wires.append(wire);
    }
}

void Node::removeWire(Wire* wire) {
    if (wire) {
        m_wires.removeOne(wire);
    }
}

QPointF Node::pos() const {
    if (!m_pins.isEmpty() && m_pins.first()) {
        return m_pins.first()->scenePos(); // 🔴 تغییر از pos به scenePos
    }
    return QPointF(0, 0);
}

LogicValue Node::resolvedValue(bool *hasConflict) const
{
    if (hasConflict) *hasConflict = false;

    bool found = false;
    LogicValue result = LogicValue::Undefined;

    for (Pin *p : m_pins) {
        if (!p || p->direction() != PinDirection::Output)
            continue; // فقط پایه‌های Output در تعیین مقدار گره نقش دارند

        const LogicValue v = p->drivenValue();
        if (v == LogicValue::Undefined)
            continue; // خروجی‌ای که هنوز مقداردهی نشده در تصمیم‌گیری شرکت نمی‌کند

        if (!found) {
            result = v;
            found = true;
        } else if (result != v) {
            // دو خروجی با مقادیر متفاوت روی یک گره → اتصال کوتاه منطقی
            if (hasConflict) *hasConflict = true;
            return LogicValue::Undefined;
        }
    }

    return found ? result : LogicValue::Undefined;
}