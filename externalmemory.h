#ifndef EXTERNALMEMORY_H
#define EXTERNALMEMORY_H

#include "digitalcomponent.h"
#include <array>

// تراشه حافظه خارجی (EEPROM/RAM) - بخش ۷.۷ مستند پروژه.
//
// پایه‌ها:
//   0-7   : A0-A7   باس آدرس ۸ بیتی (Input) - آدرس‌دهی ۲۵۶ خانه (هم‌اندازه با
//                    حافظه Flash/RAM داخلی خودِ MCU - به mcu.h مراجعه کن)
//   8-15  : D0-D7   باس داده ۸ بیتی - دوطرفه (مثل پورت‌های MCU، فاز ۸): حین
//                    نوشتن Input، حین خواندن Output
//   16    : WE      Write Enable (Input) - High=نوشتن ، Low=خواندن
//
// منطق:
//   - وقتی WE=High: پایه‌های داده Input می‌شوند. فقط روی لبه بالارونده WE
//     (Low→High - دقیقاً همان الگوی تشخیص لبه LCD::simulationTick) بایت فعلی
//     باس داده در آدرس فعلی نوشته می‌شود (نوشتن لبه‌محور، مثل استروب واقعی).
//   - وقتی WE=Low: پایه‌های داده Output می‌شوند و هر تیک بایت ذخیره‌شده در
//     آدرس فعلی را می‌رانند (خواندن پیوسته/ترکیبی، نه لبه‌محور - مطابق رفتار
//     واقعی بیشتر تراشه‌های حافظه هنگام فعال بودن خط خروجی).
//
// تصمیم صادقانه درباره Reset: چون این قطعه هم می‌تواند EEPROM (غیرفرار) و هم
// RAM (فرار) را نمایندگی کند، پیش‌فرض این پیاده‌سازی طرف EEPROM را گرفته:
// resetSimulation() محتوای حافظه را پاک نمی‌کند (فقط جهت پایه‌ها و ردیابی لبه
// WE را بازنشانی می‌کند) - دقیقاً مثل یک EEPROM واقعی که با خاموش/روشن شدن
// مجدد مدار، محتوایش را از دست نمی‌دهد.
class ExternalMemory : public DigitalComponent
{
public:
    ExternalMemory();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;
    Component* clone() const override;

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }

    void simulationTick() override;
    void resetSimulation() override;

private:
    static constexpr int kMemorySize = 256;

    std::array<quint8, kMemorySize> m_memory{};
    LogicValue m_lastWriteEnable = LogicValue::Undefined;
};

#endif // EXTERNALMEMORY_H
