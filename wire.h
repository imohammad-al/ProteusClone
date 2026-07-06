#ifndef WIRE_H
#define WIRE_H

#include <QGraphicsPathItem>
#include <QVector>
#include <QPointF>
#include "wiresegment.h"

class Pin;

class Wire : public QGraphicsPathItem
{
public:

    explicit Wire(QGraphicsItem *parent = nullptr);

    //--------------------------------------------------
    // Pins
    //--------------------------------------------------

    void setStartPin(Pin *pin);
    void setEndPin(Pin *pin);

    Pin *startPin() const;
    Pin *endPin() const;


    //--------------------------------------------------
    // Geometry
    //--------------------------------------------------

    void syncPins();

    void rebuildGeometry();

    //--------------------------------------------------
    // Points
    //--------------------------------------------------

    const QVector<QPointF> &points() const;

    void setPoints(const QVector<QPointF> &points);

    void clearPoints();

    void addPoint(const QPointF &point);

    void insertPoint(int index,
                     const QPointF &point);

    void removePoint(int index);

    QPointF point(int index) const;

    void setPoint(int index,
                  const QPointF &point);

    int pointCount() const;

    //--------------------------------------------------
    // Segment
    //--------------------------------------------------

    int segmentCount() const;

    QLineF segment(int index) const;

    void moveSegmentHorizontal(int index,
                               qreal y);

    void moveSegmentVertical(int index,
                             qreal x);

    //--------------------------------------------------
    // Utilities
    //--------------------------------------------------

    QRectF boundingRect() const override;

    QPainterPath shape() const override;



    Node *startNode() const;
    Node *endNode() const;

private:

    //--------------------------------------------------
    // Helpers
    //--------------------------------------------------

    void buildOrthogonalPath();

private:

    Pin *m_startPin = nullptr;
    Pin *m_endPin = nullptr;


    QVector<QPointF> m_points;

    QVector<WireSegment> m_segments;

    qreal m_wireWidth = 2.0;
};

#endif // WIRE_H