#include "diode.h"
#include "pin.h"
#include <QPainter>

Diode::Diode(QGraphicsItem *parent) : TwoTerminalComponent(parent) {}

QRectF Diode::boundingRect() const {
    return QRectF(-40, -25, 80, 50);
}

void Diode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    Q_UNUSED(option); Q_UNUSED(widget);

    painter->setPen(QPen(Qt::black, 2));
    painter->setBrush(Qt::NoBrush);

    // رسم سیم‌های پایه‌ها
    painter->drawLine(-30, 0, -10, 0);
    painter->drawLine(10, 0, 30, 0);

    // رسم مثلث دیود (آند به کاتد)
    QPolygonF triangle;
    triangle << QPointF(-10, -15) << QPointF(10, 0) << QPointF(-10, 15);
    painter->drawPolygon(triangle);

    // رسم خط عمودی کاتد
    painter->drawLine(10, -15, 10, 15);

    // رسم متون پویا
    painter->drawText(QPointF(-10, -20), m_label);
    painter->drawText(QPointF(-15, 30), m_model);
}

QMap<QString, ComponentProperty> Diode::getProperties() const {
    QMap<QString, ComponentProperty> props;
    props["label"] = { "نام دیود", m_label, "string", "" };
    props["model"] = { "مدل دیود", m_model, "string", "" };
    return props;
}

void Diode::setProperties(const QMap<QString, QVariant>& newValues) {
    if (newValues.contains("label")) m_label = newValues["label"].toString();
    if (newValues.contains("model")) m_model = newValues["model"].toString();
    update();
}