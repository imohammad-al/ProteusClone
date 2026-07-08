#ifndef DCSOURCE_H
#define DCSOURCE_H

#include "twoterminalcomponent.h".h"

class DCSource : public TwoTerminalComponent {
public:
    DCSource(QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    QMap<QString, ComponentProperty> getProperties() const override;
    void setProperties(const QMap<QString, QVariant>& newValues) override;

private:
    double m_voltageValue = 5.0; // ولتاژ پیش‌فرض ۵ ولت
    QString m_label = "V1";
};

#endif // DCSOURCE_H