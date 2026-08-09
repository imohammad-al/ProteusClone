#include "wireitem.h"
#include <QPainter>

WireItem::WireItem(QGraphicsItem* parent) : QGraphicsItem(parent) {
    // فعال کردن قابلیت انتخاب و فوکوس روی سیم
    setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsFocusable);
    setZValue(-1); // سیم‌ها معمولاً پشت قطعات قرار می‌گیرند
}

void WireItem::setPoints(const QVector<QPointF>& points) {
    m_points = points;
    updatePath();
}

void WireItem::addPoint(const QPointF& point) {
    m_points.append(point);
    updatePath();
}

// این تابع در زمان حرکت ماوس صدا زده می‌شود تا پیش‌نمایش سیم را آپدیت کند
void WireItem::updateActiveSegment(const QPointF& mousePos) {
    if (m_points.size() < 2) return;

    // نقطه یکی مانده به آخر (آخرین کلیک تثبیت شده کاربر)
    QPointF lastClick = m_points[m_points.size() - 2];

    // محاسبه نقطه شکست ۹۰ درجه (ترسیم عمودی-افقی یا افقی-عمودی)
    // در اینجا فرض می‌کنیم ابتدا افقی حرکت می‌کنیم سپس عمودی
    QPointF bendPoint(mousePos.x(), lastClick.y());

    // جایگذاری نقطه شکست و نقطه فعلی ماوس به عنوان دو نقطه انتهایی آرایه
    m_points[m_points.size() - 2] = bendPoint;
    m_points[m_points.size() - 1] = mousePos;

    updatePath();
}

void WireItem::updatePath() {
    prepareGeometryChange();
    m_path = QPainterPath();

    if (m_points.isEmpty()) return;

    m_path.moveTo(m_points.first());
    for (int i = 1; i < m_points.size(); ++i) {
        m_path.lineTo(m_points[i]);
    }
}

QRectF WireItem::boundingRect() const {
    // ایجاد یک حاشیه کوچک دور خطوط برای اینکه در لبه‌ها بریده نشود
    return m_path.boundingRect().adjusted(-5, -5, 5, 5);
}

QPainterPath WireItem::shape() const {
    // تبدیل خط باریک به یک مسیر ضخیم‌تر جهت راحت‌تر کلیک شدن توسط کاربر
    QPainterPathStroker stroker;
    stroker.setWidth(8);
    return stroker.createStroke(m_path);
}

void WireItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QPen pen;
    if (isSelected()) {
        pen.setColor(Qt::magenta); // رنگ سیم در حالت انتخاب شده (مثل پروتیوس)
        pen.setWidth(2);
    } else {
        pen.setColor(Qt::darkGreen); // رنگ پیش‌فرض سیم‌های دیجیتال/آنالوگ در پروتیوس
        pen.setWidth(2);
    }

    painter->setPen(pen);
    painter->drawPath(m_path);
}