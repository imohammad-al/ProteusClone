#ifndef COMPONENT_H
#define COMPONENT_H

#include <QGraphicsObject>
struct ComponentProperty {
    QString label;       // نام نمایشی (مثلاً: Resistance)
    QVariant value;      // مقدار (مثلاً: 1000 یا 5.0)
    QString type;        // نوع جعبه متن (مثلاً: "double" یا "string")
    QString unit;        // واحد اندازه گیری (مثلاً: "Ohm" یا "V")
};
class Component : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit Component(QGraphicsItem *parent = nullptr);
    int id() const;

    QString reference() const;
    QList<class Pin*> pins() const;

    void addPin(class Pin *pin);
    void updatePins();

    void updateConnections();

    virtual QMap<QString, ComponentProperty> getProperties() const = 0;
    virtual void setProperties(const QMap<QString, QVariant>& newValues) = 0;


private:
    int m_id;
    QList<class Pin*> m_pins;
    static int s_nextId;
protected:
    QVariant itemChange(GraphicsItemChange change,
                        const QVariant &value) override;
};

#endif