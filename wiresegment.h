#ifndef WIRESEGMENT_H
#define WIRESEGMENT_H

#include <QPointF>

struct WireSegment
{
    QPointF p1;
    QPointF p2;

    bool horizontal = false;
    bool vertical = false;

    WireSegment() = default;

    WireSegment(const QPointF& a,
                const QPointF& b)
        : p1(a),
        p2(b)
    {
        horizontal = qFuzzyCompare(a.y(), b.y());
        vertical   = qFuzzyCompare(a.x(), b.x());
    }
};

#endif