#ifndef DIODE_H
#define DIODE_H

#include "twoterminalcomponent.h".h"

class Diode : public TwoTerminalComponent {
public:
    Diode(QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    QMap<QString, ComponentProperty> getProperties() const override;
    void setProperties(const QMap<QString, QVariant>& newValues) override;

private:
    QString m_label = "D1";
    QString m_model = "1N4148"; // مدل دیود به عنوان یک ویژگی متنی
};

#endif // DIODE_H