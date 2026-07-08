#include "capacitor.h"
#include "pin.h"
#include <QPainter>

Capacitor::Capacitor(QGraphicsItem *parent)
    : TwoTerminalComponent(parent) // تغییر دادیم تا مثل مقاومت از کلاس دوپایه‌ای ارث ببرد
{
    // اگر کلاس TwoTerminalComponent خودش پین‌ها را نمی‌سازد، این دو خط را نگه دارید:
    // positivePin->setPos(QPointF(-30, 0));
    // negativePin->setPos(QPointF(30, 0));
}
QRectF Capacitor::boundingRect() const {
    // محدوده کلیک و رسم خازن
    return QRectF(-40, -20, 80, 40);
}

void Capacitor::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    Q_UNUSED(option); Q_UNUSED(widget);

    painter->setPen(QPen(Qt::black, 2));

    // رسم سیم‌های پایه‌ها
    painter->drawLine(-30, 0, -10, 0);
    painter->drawLine(10, 0, 30, 0);

    // رسم دو خط موازی صفحات خازن
    painter->drawLine(-10, -15, -10, 15);
    painter->drawLine(10, -15, 10, 15);

    // رسم نام و مقدار پویا روی بوم
    painter->drawText(QPointF(-10, -20), m_label);
    painter->drawText(QPointF(-10, 30), QString::number(m_capacitanceValue) + " F");
}

// معرفی ویژگی‌های خازن به پنجره پویا
QMap<QString, ComponentProperty> Capacitor::getProperties() const {
    QMap<QString, ComponentProperty> props;
    props["label"] = { "نام خازن", m_label, "string", "" };
    props["capacitance"] = { "ظرفیت خازن", m_capacitanceValue, "double", "F" };
    return props;
}

// اعمال تغییرات پنجره پویا به خازن
void Capacitor::setProperties(const QMap<QString, QVariant>& newValues) {
    if (newValues.contains("label")) m_label = newValues["label"].toString();
    if (newValues.contains("capacitance")) m_capacitanceValue = newValues["capacitance"].toDouble();
    update();
}