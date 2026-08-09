#include "wire.h"
#include "pin.h"
#include "node.h"
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionGraphicsItem>

Wire::Wire(Pin* startPin, Pin* endPin, QGraphicsItem* parent)
    : QGraphicsPathItem(parent), m_startPin(startPin), m_endPin(endPin)
{
    setPen(QPen(Qt::darkGreen, 2)); // سیم‌ها در پروتئوس معمولاً سبز تیره یا آبی هستند
    setZValue(-1); // سیم‌ها همیشه پشت قطعات قرار بگیرند
    setFlag(QGraphicsItem::ItemIsSelectable); // برای این‌که بشود سیم را انتخاب/حذف کرد (بخش ۵.۵ مستند)
    rebuildGeometry();
}

Wire::~Wire() {
    // قطع ارتباطات در صورت حذف سیم
}

void Wire::rebuildGeometry() {
    if (!m_startPin || !m_endPin) return;

    // گرفتن موقعیت پین‌ها در فضای بوم (Scene Coords)
    QPointF p1 = m_startPin->scenePos();
    QPointF p2 = m_endPin->scenePos();

    m_points.clear();
    m_points << p1;

    // الگوریتم رسم سیم ۹۰ درجه (Orthogonal Routing):
    // یک نقطه واسط برای شکستن سیم ایجاد می‌کنیم تا سیم مورب رسم نشود
    QPointF midPoint(p2.x(), p1.y());
    m_points << midPoint;
    m_points << p2;

    // ساخت مسیر گرافیکی بر اساس نقاط
    QPainterPath path;
    path.moveTo(m_points.first());
    for (int i = 1; i < m_points.size(); ++i) {
        path.lineTo(m_points.at(i));
    }

    setPath(path);
}

QRectF Wire::boundingRect() const {
    return path().boundingRect().adjusted(-5, -5, 5, 5);
}

void Wire::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(widget);

    // نمایش زنده وضعیت منطقی سیم حین شبیه‌سازی (بخش ۸.۲ مستند پروژه):
    // وقتی شبیه‌سازی در حال اجرا نیست، همه پایه‌های Output مقدار Undefined دارند
    // (به‌خاطر resetSimulation()) پس این بخش خودکار به رنگ پیش‌فرض برمی‌گردد.
    QPen wirePen = pen();

    if (m_startPin && m_startPin->node()) {
        bool conflict = false;
        const LogicValue value = m_startPin->node()->resolvedValue(&conflict);

        if (conflict) {
            wirePen.setColor(Qt::magenta); // تداخل/اتصال کوتاه منطقی روی این گره
        } else if (value == LogicValue::High) {
            wirePen.setColor(Qt::red);
        } else if (value == LogicValue::Low) {
            wirePen.setColor(QColor(30, 30, 200));
        }
        // در حالت Undefined رنگ پیش‌فرض (سبز تیره) حفظ می‌شود
    }

    // نشانگر انتخاب: وقتی سیم انتخاب شده، ضخیم‌تر و نقطه‌چین رسم می‌شود (بخش ۵.۵ مستند)
    if (option && (option->state & QStyle::State_Selected)) {
        wirePen.setWidth(wirePen.width() + 2);
        wirePen.setStyle(Qt::DashLine);
    }

    painter->setPen(wirePen);
    painter->drawPath(path());
}