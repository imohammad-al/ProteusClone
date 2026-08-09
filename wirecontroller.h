#ifndef WIRECONTROLLER_H
#define WIRECONTROLLER_H

#include <QObject>
#include <QVector>
#include <QPointF>

class CircuitScene;
class Wire;
class Pin;

class WireController : public QObject
{
    Q_OBJECT

public:

    explicit WireController(CircuitScene *scene);

    void mousePress(const QPointF &scenePos,
                    Pin *pin);

    void mouseMove(const QPointF &scenePos);

    void cancel();

    bool drawing() const;

private:

    void beginWire(Pin *pin);

    void finishWire(Pin *pin);

    void addCorner(const QPointF &scenePos);

private:

    CircuitScene *m_scene;

    Wire *m_wire = nullptr;

    Pin *m_startPin = nullptr;

    QVector<QPointF> m_points;

    bool m_drawing = false;
};

#endif