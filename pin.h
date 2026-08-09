#ifndef PIN_H
#define PIN_H

#include <QGraphicsObject>
#include <QUuid>
#include <QList>

class Wire;
class Node;

// جهت الکتریکی/منطقی پایه - برای موتور شبیه‌سازی دیجیتال استفاده می‌شود.
// پیش‌فرض Bidirectional است تا قطعات آنالوگ قدیمی (مقاومت، خازن، دیود و...)
// که این مقدار را صراحتاً تنظیم نمی‌کنند، بدون تغییر رفتار قبلی باقی بمانند
// و در محاسبه گره‌های دیجیتال شرکت نکنند.
enum class PinDirection {
    Input,
    Output,
    Bidirectional
};

// مقدار منطقی یک پایه/گره در شبیه‌سازی دیجیتال (بخش ۶.۴ و ۸.۲ مستند پروژه)
enum class LogicValue {
    Low,
    High,
    Undefined
};

class Pin : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit Pin(QGraphicsItem *parent = nullptr);

    QUuid id() const { return m_id; }

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

    // متدهای مدیریت اتصالات پین بر اساس پروپوزال
    void addWire(Wire *wire);
    void removeWire(Wire *wire);
    QList<Wire*> wires() const { return m_wires; }

    Node *node() const { return m_node; }
    void setNode(Node *node);

    // تابعی که سیم‌های متصل به این پین را وادار به کش‌آمدن می‌کند
    void updateWires();

    // --- شبیه‌سازی دیجیتال ---
    PinDirection direction() const { return m_direction; }
    void setDirection(PinDirection direction) { m_direction = direction; }

    // فقط برای پایه‌های Output معنا دارد: مقداری که این پایه به گره خودش تحمیل می‌کند.
    // Node::resolvedValue() فقط پایه‌های Output را برای تعیین مقدار گره در نظر می‌گیرد.
    LogicValue drivenValue() const { return m_drivenValue; }
    void setDrivenValue(LogicValue value) { m_drivenValue = value; }

private:
    QUuid m_id;
    QList<Wire*> m_wires;
    Node *m_node = nullptr;

    PinDirection m_direction = PinDirection::Bidirectional;
    LogicValue m_drivenValue = LogicValue::Undefined;
};

#endif // PIN_H