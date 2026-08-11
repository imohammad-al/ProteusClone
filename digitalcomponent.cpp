#include "digitalcomponent.h"
#include "node.h"

DigitalComponent::DigitalComponent(QGraphicsItem *parent)
    : Component(parent)
{
}

LogicValue DigitalComponent::inputValue(int index) const
{
    Pin *p = pin(index);
    if (!p || !p->node())
        return LogicValue::Undefined;
    return p->node()->resolvedValue();
}

void DigitalComponent::simulationTick()
{
    Pin *out = pin(outputPinIndex());

    // ۱. اگر مقدار معلقی از قبل زمان‌بندی شده و شمارش معکوسش تمام شده، همین حالا اعمالش کن
    if (m_pendingCountdown == 0) {
        if (out) out->setDrivenValue(m_pendingValue);
        m_pendingCountdown = -1;
    } else if (m_pendingCountdown > 0) {
        m_pendingCountdown--;
    }

    // ۲. مقدار جدید را بر اساس ورودی‌های فعلی محاسبه و برای اعمال تاخیردار زمان‌بندی کن.
    //    نکته: اگر همین الان یک تغییر دیگر در حال شمارش معکوس باشد، منتظر اعمال آن می‌مانیم
    //    (ساده‌سازی عمدی به‌جای صف رویداد کامل - برای گیت‌های ترکیبی معمولی کافی است).
    if (m_pendingCountdown < 0 && out) {
        const LogicValue newValue = computeOutput();
        if (newValue != out->drivenValue()) {
            m_pendingValue = newValue;
            m_pendingCountdown = m_delaySteps;
        }
    }
}

void DigitalComponent::resetSimulation()
{
    m_pendingCountdown = -1;
    m_pendingValue = LogicValue::Undefined;

    if (Pin *out = pin(outputPinIndex()))
        out->setDrivenValue(LogicValue::Undefined);
}
