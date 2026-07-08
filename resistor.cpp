#include "resistor.h"

Resistor::Resistor(QGraphicsItem *parent)
    : TwoTerminalComponent(parent)
{

}

QRectF Resistor::boundingRect() const
{
    return QRectF(-45,-20,90,40);
}

void Resistor::paint(QPainter *painter,
                     const QStyleOptionGraphicsItem *,
                     QWidget *)

{
    painter->setRenderHint(QPainter::Antialiasing, true);

    painter->setPen(QPen(Qt::black,2));

    painter->setBrush(Qt::NoBrush);

    painter->drawLine(-40,0,-25,0);

    painter->drawRect(-25,-10,50,20);

    painter->drawLine(25,0,40,0);


    // رسم نام قطعه (مثلاً R1 یا نام جدیدی که کاربر وارد کرده)
    painter->drawText(QPointF(-10, -20), m_label);

    // رسم مقدار مقاومت (مثلاً 1000 Ω) - اختیاری اگر دوست دارید نمایش داده شود
    QString resistanceText = QString::number(m_resistanceValue) + " Ω";
    painter->drawText(QPointF(-10, 30), resistanceText);
}


QMap<QString, ComponentProperty> Resistor::getProperties() const {
    QMap<QString, ComponentProperty> props;
    props["label"] = { "نام قطعه", m_label, "string", "" };
    props["resistance"] = { "مقدار مقاومت", m_resistanceValue, "double", "Ω" };
    return props;
}

void Resistor::setProperties(const QMap<QString, QVariant>& newValues) {
    if (newValues.contains("label"))
        m_label = newValues["label"].toString();
    if (newValues.contains("resistance"))
        m_resistanceValue = newValues["resistance"].toDouble();

    update(); // این تابع ظاهر قطعه روی بوم را دوباره رسم می‌کند تا متن جدید دیده شود
}