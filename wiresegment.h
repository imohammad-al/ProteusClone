#ifndef WIRESEGMENT_H
#define WIRESEGMENT_H

#include <QPointF>

struct WireSegment
{
    QPointF start;
    QPointF end;

    bool horizontal = false;
    bool vertical = false;

    int firstPointIndex = -1;
    int secondPointIndex = -1;

    WireSegment() = default;

    WireSegment(const QPointF &a,
                const QPointF &b)
        : start(a),
        end(b)
    {
        horizontal = qFuzzyCompare(a.y(), b.y());
        vertical   = qFuzzyCompare(a.x(), b.x());
    }
};

#endif