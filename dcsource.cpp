#include "dcsource.h"
#include "pin.h"
#include <QPainter>

DCSource::DCSource(QGraphicsItem *parent) : TwoTerminalComponent(parent) {}

QRectF DCSource::boundingRect() const {
    return QRectF(-40, -25, 80, 50);
}

void DCSource::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    Q_UNUSED(option); Q_UNUSED(widget);

    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(Qt::NoBrush);

    // رسم سیم‌های اتصال به دایره
    painter->drawLine(-30, 0, -15, 0);
    painter->drawLine(15, 0, 30, 0);

    // رسم دایره اصلی منبع
    painter->drawEllipse(QPointF(0, 0), 15, 15);

    // رسم علامت + و - درون دایره
    painter->drawText(QPointF(-12, 5), "+");
    painter->drawText(QPointF(3, 4), "-");

    // رسم نام و مقدار ولتاژ در اطراف دایره
    painter->drawText(QPointF(-10, -20), m_label);
    painter->drawText(QPointF(-12, 32), QString::number(m_voltageValue) + " V");
}

QMap<QString, ComponentProperty> DCSource::getProperties() const {
    QMap<QString, ComponentProperty> props;
    props["label"] = { "نام منبع", m_label, "string", "" };
    props["voltage"] = { "ولتاژ منبع", m_voltageValue, "double", "V" };
    return props;
}

void DCSource::setProperties(const QMap<QString, QVariant>& newValues) {
    if (newValues.contains("label")) m_label = newValues["label"].toString();
    if (newValues.contains("voltage")) m_voltageValue = newValues["voltage"].toDouble();
    update();
}