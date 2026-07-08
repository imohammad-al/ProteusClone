#ifndef CAPACITOR_H
#define CAPACITOR_H

#include "twoterminalcomponent.h"

class Capacitor : public TwoTerminalComponent {
public:
   // Capacitor();
    Capacitor(QGraphicsItem *parent = nullptr);

    // متدهای استاندارد گرافیکی Qt که احتمالاً در Resistor هم دارید
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    // متدهای مدیریت ویژگی‌ها که خودمان ساختیم
    QMap<QString, ComponentProperty> getProperties() const override;
    void setProperties(const QMap<QString, QVariant>& newValues) override;

private:
    double m_capacitanceValue = 0.00001; // ۱۰ میکروفاراد پیش‌فرض
    QString m_label = "C1";
};

#endif // CAPACITOR_H