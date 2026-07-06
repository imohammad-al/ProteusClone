#ifndef NET_H
#define NET_H

#include <QObject>
#include <QList>
#include <QString>

class Pin;
class Wire;

class Net : public QObject
{
    Q_OBJECT

public:
    explicit Net(QObject *parent = nullptr);

    void addPin(Pin *pin);
    void removePin(Pin *pin);

    void addWire(Wire *wire);
    void removeWire(Wire *wire);

    const QList<Pin*>& pins() const;
    const QList<Wire*>& wires() const;

    QString name() const;
    void setName(const QString &name);

private:
    QList<Pin*> m_pins;
    QList<Wire*> m_wires;

    QString m_name;
};

#endif