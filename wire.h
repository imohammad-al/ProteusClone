#ifndef WIRE_H
#define WIRE_H

#include <QGraphicsPathItem>
#include <QVector>
#include <QPointF>

#include "wiresegment.h"

class Pin;
class Node;
class QGraphicsSceneMouseEvent;

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
    // Nodes
    //--------------------------------------------------

    Node *startNode() const;
    Node *endNode() const;

    void setStartNode(Node *node);
    void setEndNode(Node *node);

    //--------------------------------------------------
    // Temporary Endpoints
    //--------------------------------------------------

    void setStartPoint(const QPointF &point);
    void setEndPoint(const QPointF &point);

    QPointF startPoint() const;
    QPointF endPoint() const;

    //--------------------------------------------------
    // Geometry
    //--------------------------------------------------

    void syncPins();

    void updatePosition();

    void rebuildGeometry();

    //--------------------------------------------------
    // Polyline Points
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
    // Segments
    //--------------------------------------------------

    const QVector<WireSegment> &segments() const;

    int segmentCount() const;

    QLineF segment(int index) const;

    void moveSegmentHorizontal(int index,
                               qreal y);

    void moveSegmentVertical(int index,
                             qreal x);



    //--------------------------------------------------
    // Highlight
    //--------------------------------------------------

    void setHighlighted(bool value);

    bool highlighted() const;

    //--------------------------------------------------
    // Graphics
    //--------------------------------------------------

    QRectF boundingRect() const override;

    QPainterPath shape() const override;

    int segmentAt(const QPointF &scenePos) const;

    bool isSegmentHorizontal(int index) const;

protected:

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:

    void buildOrthogonalPath();

    void buildSegments();

private:

    Pin *m_startPin = nullptr;
    Pin *m_endPin = nullptr;

    Node *m_startNode = nullptr;
    Node *m_endNode = nullptr;

    QPointF m_startPoint;
    QPointF m_endPoint;

    QVector<QPointF> m_points;

    QVector<WireSegment> m_segments;

    bool m_highlighted = false;

    qreal m_wireWidth = 2.0;

    int m_dragSegment = -1;

    bool m_draggingSegment = false;

    QPointF m_lastMousePos;
};

#endif