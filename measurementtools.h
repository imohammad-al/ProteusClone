#ifndef MEASUREMENTTOOLS_H
#define MEASUREMENTTOOLS_H

#include "component.h"
#include "pin.h" // برای LogicValue (فاز ۱۶: باگ واقعی که با اولین کامپایل واقعی این
                  // پروژه کشف شد - قبلاً فقط با شانسِ ترتیب include در واحدهای
                  // ترجمه‌ی دیگر کامپایل می‌شد)
#include <QVector>

// --- پراب منطقی ---
// یک قطعه صرفاً نمایشی (مثل LED) که مقدار لحظه‌ای گره متصل به پایه ورودی‌اش را
// به‌صورت متن H/L/? نشان می‌دهد. هیچ چیزی را نمی‌راند، فقط می‌خواند.
class LogicProbe : public Component {
public:
    LogicProbe();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;
    Component* clone() const override;

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }
};

// --- ولت‌متر دیجیتال (نسخه قدیمی/میراثی - پیش از وجود موتور آنالوگ) ---
// چون در زمان ساخت این کلاس موتور شبیه‌سازی فقط سطح منطقی (Low/High/Undefined)
// داشت نه ولتاژ واقعی، این قطعه مقدار منطقی گره را بر اساس یک "ولتاژ منبع تغذیه"
// قابل‌تنظیم (پیش‌فرض ۵ ولت) به یک عدد نمایشی نگاشت می‌کند - یک تقریب صادقانه،
// نه اندازه‌گیری آنالوگ واقعی. از فاز اضافه‌شدن sim/analogsolver.h به بعد،
// AnalogVoltmeter/AnalogAmmeter پایین همین فایل نسخه‌های *واقعی* این ابزارها
// هستند؛ این کلاس فقط برای سازگاری با پروژه‌های قدیمی ذخیره‌شده نگه داشته شده
// و دست نخورده باقی مانده (تغییرش می‌توانست فرمت ذخیره/بازیابی قدیمی را بشکند).
class DigitalVoltmeter : public Component {
public:
    DigitalVoltmeter();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;
    Component* clone() const override;

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }
};

// --- ولت‌متر آنالوگ واقعی (بخش ۹.۲ مستند) ---
// دو پایه دارد؛ ولتاژ V(pin0)-V(pin1) را نشان می‌دهد (برای حالت «نسبت به زمین»
// که مستند اشاره می‌کند، کافیست pin1 را به یک Ground وصل کنی). خودِ این قطعه
// در تحلیل مداری sim/analogsolver.h هیچ نقشی ندارد (ولت‌متر ایده‌آل = مقاومت
// ورودی بی‌نهایت = هیچ اثری روی مدار نمی‌گذارد)؛ فقط بعد از حل شدن مدار،
// Node::voltage() دو پایه‌اش را می‌خواند. اگر هرکدام از دو گره اصلاً محاسبه
// نشده باشند (چون آن بخش از مدار به هیچ Groundی وصل نیست) طبق مستند «ERR»
// نشان می‌دهد.
class AnalogVoltmeter : public Component {
public:
    AnalogVoltmeter();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;
    Component* clone() const override;

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }
};

// --- آمپرمتر آنالوگ واقعی (بخش ۹.۲ مستند) ---
// روی یک سیم به‌صورت سری قرار می‌گیرد (۲ پایه). برخلاف ولت‌متر، این قطعه واقعاً
// روی مدار اثر دارد: sim/analogsolver.h آن را دقیقاً مثل یک منبع ولتاژ ایده‌آل
// با ولتاژ ۰ مدل می‌کند (افت ولتاژ صفر روی آمپرمتر ایده‌آل) و جریان مجهولِ
// کمکیِ همان منبع در روش MNA، همان جریانی است که از آمپرمتر عبور کرده -
// همان ترفند استانداردی که شبیه‌سازهای اسپایس برای اندازه‌گیری جریان استفاده
// می‌کنند. مقدار را با setMeasuredCurrent() از AnalogSolver دریافت می‌کند.
class AnalogAmmeter : public Component {
public:
    AnalogAmmeter();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;
    Component* clone() const override;

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }

    // فقط توسط AnalogSolver صدا زده می‌شود؛ آمپر (نه میلی‌آمپر) واحدش است.
    void setMeasuredCurrent(double amperes);
    void clearMeasurement();

private:
    double m_measuredCurrent = 0.0;
    bool m_hasMeasurement = false;
};

// --- اسیلوسکوپ دو کاناله ---
// تاریخچه مقدار منطقی هر یک از دو ورودی را در طول اجرای شبیه‌سازی ذخیره و به‌صورت
// موج مستطیلی (Step Waveform) رسم می‌کند - دقیقاً مثل رفتار یک اسیلوسکوپ دیجیتال
// واقعی روی سیگنال‌های منطقی (بخش ۹.۴ مستند: حداقل دو کانال).
class Oscilloscope : public Component {
public:
    Oscilloscope();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;
    Component* clone() const override;

    bool isValid() override { return true; }
    SimulationElement simulationModel() override { return SimulationElement(); }

    // توسط SimulationEngine در هر گام شبیه‌سازی صدا زده می‌شود تا نمونه فعلی هر کانال ثبت شود
    void sampleChannels();
    void clearHistory();

private:
    static constexpr int kMaxSamples = 64;
    QVector<LogicValue> m_channel1History;
    QVector<LogicValue> m_channel2History;
};

#endif // MEASUREMENTTOOLS_H
