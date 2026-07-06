#include "wire.h"

#include "pin.h"
#include "node.h"
#include "net.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPen>

////////////////////////////////////////////////////////////

Wire::Wire(QGraphicsItem *parent)
    : QGraphicsPathItem(parent)
{
    setPen(QPen(Qt::blue,2));

    setBrush(Qt::NoBrush);

    setAcceptedMouseButtons(Qt::LeftButton);

    setFlag(QGraphicsItem::ItemIsSelectable,true);

    setZValue(-100);
}

////////////////////////////////////////////////////////////
// Pins
////////////////////////////////////////////////////////////

Pin *Wire::startPin() const
{
    return m_startPin;
}

Pin *Wire::endPin() const
{
    return m_endPin;
}

void Wire::setStartPin(Pin *pin)
{
    if(m_startPin == pin)
        return;

    m_startPin = pin;

    if(pin)
    {
        pin->addWire(this);

        m_startNode = pin->node();

        if(m_startNode && m_startNode->net())
            m_startNode->net()->addWire(this);

        m_startPoint = pin->scenePos();
    }

    rebuildGeometry();
}

void Wire::setEndPin(Pin *pin)
{
    if(m_endPin == pin)
        return;

    m_endPin = pin;

    if(pin)
    {
        pin->addWire(this);

        m_endNode = pin->node();

        if(m_endNode && m_endNode->net())
            m_endNode->net()->addWire(this);

        m_endPoint = pin->scenePos();
    }

    rebuildGeometry();
}

////////////////////////////////////////////////////////////
// Nodes
////////////////////////////////////////////////////////////

Node *Wire::startNode() const
{
    return m_startNode;
}

Node *Wire::endNode() const
{
    return m_endNode;
}

void Wire::setStartNode(Node *node)
{
    m_startNode = node;
}

void Wire::setEndNode(Node *node)
{
    m_endNode = node;
}

////////////////////////////////////////////////////////////
// Temporary Points
////////////////////////////////////////////////////////////

void Wire::setStartPoint(const QPointF &point)
{
    m_startPoint = point;
    rebuildGeometry();
}

void Wire::setEndPoint(const QPointF &point)
{
    m_endPoint = point;
    rebuildGeometry();
}

QPointF Wire::startPoint() const
{
    return m_startPoint;
}

QPointF Wire::endPoint() const
{
    return m_endPoint;
}

////////////////////////////////////////////////////////////
// Synchronize Pins
////////////////////////////////////////////////////////////

void Wire::syncPins()
{
    if(m_startPin)
        m_startPoint = m_startPin->scenePos();

    if(m_endPin)
        m_endPoint = m_endPin->scenePos();
}

void Wire::updatePosition()
{
    syncPins();
    rebuildGeometry();
}

////////////////////////////////////////////////////////////
// Points
////////////////////////////////////////////////////////////

const QVector<QPointF> &Wire::points() const
{
    return m_points;
}

void Wire::setPoints(const QVector<QPointF> &points)
{
    m_points = points;
    rebuildGeometry();
}

void Wire::clearPoints()
{
    m_points.clear();
    rebuildGeometry();
}

void Wire::addPoint(const QPointF &point)
{
    m_points.append(point);
    rebuildGeometry();
}

void Wire::insertPoint(int index,
                       const QPointF &point)
{
    if(index < 0 || index > m_points.size())
        return;

    m_points.insert(index, point);

    rebuildGeometry();
}

void Wire::removePoint(int index)
{
    if(index < 0 || index >= m_points.size())
        return;

    m_points.removeAt(index);

    rebuildGeometry();
}

QPointF Wire::point(int index) const
{
    if(index < 0 || index >= m_points.size())
        return QPointF();

    return m_points[index];
}

void Wire::setPoint(int index,
                    const QPointF &point)
{
    if(index < 0 || index >= m_points.size())
        return;

    m_points[index] = point;

    rebuildGeometry();
}

int Wire::pointCount() const
{
    return m_points.size();
}

////////////////////////////////////////////////////////////
// memments
////////////////////////////////////////////////////////////

const QVector<WireSegment> &Wire::segments() const
{
    return m_segments;
}

int Wire::segmentCount() const
{
    return m_segments.size();
}

QLineF Wire::segment(int index) const
{
    if(index < 0 || index >= m_segments.size())
        return QLineF();

    return QLineF(m_segments[index].start,
                  m_segments[index].end);
}

void Wire::moveSegmentHorizontal(int index,
                                 qreal y)
{
    if(index < 0 || index >= m_segments.size())
        return;

    WireSegment &seg = m_segments[index];

    int p1 = seg.firstPointIndex;
    int p2 = seg.secondPointIndex;

    if(p1 >= 0 && p1 < m_points.size())
        m_points[p1].setY(y);

    if(p2 >= 0 && p2 < m_points.size())
        m_points[p2].setY(y);

    rebuildGeometry();
}


void Wire::moveSegmentVertical(int index,
                               qreal x)
{
    if(index < 0 || index >= m_segments.size())
        return;

    WireSegment &seg = m_segments[index];

    int p1 = seg.firstPointIndex;
    int p2 = seg.secondPointIndex;

    if(p1 >= 0 && p1 < m_points.size())
        m_points[p1].setX(x);

    if(p2 >= 0 && p2 < m_points.size())
        m_points[p2].setX(x);

    rebuildGeometry();
}

/*int Wire::segmentAt(const QPointF &scenePos) const
{
    const qreal tol = 6.0;

    for(int i = 0; i < m_segments.size(); ++i)
    {
        QLineF line(m_segments[i].p1,
                    m_segments[i].p2);

        QPointF p;

        if(line.length() < 0.1)
            continue;

        qreal d = line.distanceToPoint(scenePos);

        if(d <= tol)
            return i;
    }

    return -1;
} */

bool Wire::isSegmentHorizontal(int index) const
{
    if(index < 0 || index >= m_segments.size())
        return false;

    return m_segments[index].horizontal;
}
////////////////////////////////////////////////////////////
// Highlight
////////////////////////////////////////////////////////////

void Wire::setHighlighted(bool value)
{
    m_highlighted = value;

    if(value)
        setPen(QPen(Qt::red,3));
    else
        setPen(QPen(Qt::blue,2));
}

bool Wire::highlighted() const
{
    return m_highlighted;
}

////////////////////////////////////////////////////////////
// Geometry
////////////////////////////////////////////////////////////

void Wire::buildSegments()
{
    m_segments.clear();

    if(m_points.size() < 2)
        return;

    for(int i = 0; i < m_points.size() - 1; ++i)
    {
        WireSegment seg(
            m_points[i],
            m_points[i + 1]);

        seg.firstPointIndex = i;
        seg.secondPointIndex = i + 1;

        seg.horizontal =
            qAbs(seg.start.y() - seg.end.y()) < 0.01;

        seg.vertical =
            qAbs(seg.start.x() - seg.end.x()) < 0.01;

        m_segments.append(seg);
    }
}
void Wire::buildOrthogonalPath()
{
    QPainterPath path;

    path.moveTo(m_startPoint);
    path.lineTo(m_endPoint);

    setPath(path);

    m_points.clear();
    m_points << m_startPoint
             << m_endPoint;

    buildSegments();
}
void Wire::rebuildGeometry()
{
    if(m_startPin)
        m_startPoint = m_startPin->scenePos();

    if(m_endPin)
        m_endPoint = m_endPin->scenePos();

    // فعلاً فقط یک خط مستقیم بین ابتدا و انتها
    QPainterPath path;

    path.moveTo(m_startPoint);
    path.lineTo(m_endPoint);

    setPath(path);

    // بروزرسانی نقاط
    m_points.clear();
    m_points << m_startPoint
             << m_endPoint;

    // بروزرسانی سگمنت‌ها
    buildSegments();
}
////////////////////////////////////////////////////////////
// Graphics
////////////////////////////////////////////////////////////

QRectF Wire::boundingRect() const
{
    return QGraphicsPathItem::boundingRect().adjusted(
        -6,
        -6,
        6,
        6);
}

QPainterPath Wire::shape() const
{
    QPainterPathStroker stroker;

    stroker.setWidth(10);

    return stroker.createStroke(path());
}

////////////////////////////////////////////////////////////
// Mouse
////////////////////////////////////////////////////////////

void Wire::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_dragSegment = -1;

    QPointF p = event->scenePos();

    for(int i = 0; i < m_segments.size(); ++i)
    {
        QLineF line(m_segments[i].start,
                    m_segments[i].end);

        QPointF v = line.p2() - line.p1();

        double len2 = v.x()*v.x() + v.y()*v.y();

        if(len2 < 1e-6)
            continue;

        double t =
            ((p.x()-line.x1())*v.x() +
             (p.y()-line.y1())*v.y()) / len2;

        t = qBound(0.0, t, 1.0);

        QPointF proj(
            line.x1() + t*v.x(),
            line.y1() + t*v.y());

        if(QLineF(proj, p).length() < 6.0)
        {
            m_dragSegment = i;
            break;
        }
    }

    if(m_dragSegment != -1)
    {
        m_draggingSegment = true;
        m_lastMousePos = event->scenePos();
    }

    QGraphicsPathItem::mousePressEvent(event);
}
void Wire::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if(!m_draggingSegment)
    {
        QGraphicsPathItem::mouseMoveEvent(event);
        return;
    }

    QPointF delta = event->scenePos() - m_lastMousePos;

    if(m_dragSegment >= 0 &&
        m_dragSegment < m_segments.size())
    {
        WireSegment &seg = m_segments[m_dragSegment];

        if(seg.horizontal)
        {
            qreal y = seg.start.y() + delta.y();
            moveSegmentHorizontal(m_dragSegment, y);
        }
        else if(seg.vertical)
        {
            qreal x = seg.start.x() + delta.x();
            moveSegmentVertical(m_dragSegment, x);
        }
    }

    m_lastMousePos = event->scenePos();
}

void Wire::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    m_draggingSegment = false;

    m_dragSegment = -1;

    QGraphicsPathItem::mouseReleaseEvent(event);
}
