#ifndef DIGITALLOGIC_H
#define DIGITALLOGIC_H

#include "pin.h" // برای LogicValue

// عملگرهای منطقی پایه که همه گیت‌ها از آن‌ها استفاده می‌کنند.
// قانون مشترک: اگر یک مقدار Undefined باشد ولی مقدار دیگر به‌تنهایی نتیجه را
// مشخص کند (مثلاً یک ورودی AND برابر Low است)، نتیجه همچنان قطعی است -
// دقیقاً مثل رفتار گیت‌های واقعی دیجیتال.

inline LogicValue logicNot(LogicValue a)
{
    if (a == LogicValue::Undefined) return LogicValue::Undefined;
    return (a == LogicValue::High) ? LogicValue::Low : LogicValue::High;
}

inline LogicValue logicAnd(LogicValue a, LogicValue b)
{
    if (a == LogicValue::Low || b == LogicValue::Low) return LogicValue::Low;
    if (a == LogicValue::Undefined || b == LogicValue::Undefined) return LogicValue::Undefined;
    return LogicValue::High;
}

inline LogicValue logicOr(LogicValue a, LogicValue b)
{
    if (a == LogicValue::High || b == LogicValue::High) return LogicValue::High;
    if (a == LogicValue::Undefined || b == LogicValue::Undefined) return LogicValue::Undefined;
    return LogicValue::Low;
}

inline LogicValue logicNand(LogicValue a, LogicValue b)
{
    return logicNot(logicAnd(a, b));
}

inline LogicValue logicXor(LogicValue a, LogicValue b)
{
    if (a == LogicValue::Undefined || b == LogicValue::Undefined) return LogicValue::Undefined;
    return (a != b) ? LogicValue::High : LogicValue::Low;
}

#endif // DIGITALLOGIC_H
