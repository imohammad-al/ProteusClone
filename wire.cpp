#include "wire.h"
#include "pin.h"
#include "node.h"
#include <QGraphicsSceneMouseEvent>
#include <QPen>

Wire::Wire(QGraphicsItem *parent)
    : QGraphicsLineItem(parent)
{
    setPen(QPen(Qt::blue,2));

    setAcceptedMouseButtons(Qt::LeftButton);

    setFlag(QGraphicsItem::ItemIsSelectable,false);

    setZValue(-100);
}

//////////////////////////////////////////////////////////
// Pins
//////////////////////////////////////////////////////////

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

        if(m_startNode)
            m_startNode->addWire(this);
    }

    updatePosition();
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

        if(m_endNode)
            m_endNode->addWire(this);
    }

    updatePosition();
}

//////////////////////////////////////////////////////////
// Nodes
//////////////////////////////////////////////////////////


Node *Wire::startNode() const
{
    return m_startPin ? m_startPin->node() : nullptr;
}

Node *Wire::endNode() const
{
    return m_endPin ? m_endPin->node() : nullptr;
}
//////////////////////////////////////////////////////////
// Geometry
//////////////////////////////////////////////////////////

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

void Wire::updatePosition()
{
    if(m_startPin)
        m_startPoint = m_startPin->scenePos();

    if(m_endPin)
        m_endPoint = m_endPin->scenePos();

    rebuildGeometry();
}

//////////////////////////////////////////////////////////
// Future Path Support
//////////////////////////////////////////////////////////

void Wire::rebuildGeometry()
{
    // فعلاً فقط یک خط رسم می‌کنیم

    QLineF line(m_startPoint,m_endPoint);

    setLine(line);
}

const QVector<QPointF> &Wire::points() const
{
    return m_points;
}

void Wire::setPoints(const QVector<QPointF> &points)
{
    m_points = points;

    if(points.size() >= 2)
    {
        m_startPoint = points.first();

        m_endPoint = points.last();
    }

    rebuildGeometry();
}

void Wire::addPoint(const QPointF &point)
{
    m_points.append(point);

    if(m_points.size() >= 2)
    {
        m_startPoint = m_points.first();

        m_endPoint = m_points.last();
    }

    rebuildGeometry();
}

void Wire::setHighlighted(bool value)
{
    m_highlighted=value;

    if(value)
    {
        setPen(QPen(Qt::red,3));
    }
    else
    {
        setPen(QPen(Qt::blue,2));
    }
}

bool Wire::highlighted() const
{
    return m_highlighted;
}

void Wire::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(m_startNode)
    {
        m_startNode->highlight(
            !m_startNode->highlighted());
    }

    QGraphicsLineItem::mousePressEvent(event);
}

void Wire::setStartNode(Node *node)
{
    m_startNode = node;
}

void Wire::setEndNode(Node *node)
{
    m_endNode = node;
}
