#ifndef WIREITEM_H
#define WIREITEM_H

#include <QGraphicsItem>
#include <QVector>
#include <QPointF>
#include <QPainterPath>
#include <QPen>

class WireItem : public QGraphicsItem {
public:
    WireItem(QGraphicsItem* parent = nullptr);

    // مدیریت نقاط (Corners)
    QVector<QPointF> points() const { return m_points; }
    void setPoints(const QVector<QPointF>& points);
    void addPoint(const QPointF& point);

    // به‌روزرسانی دینامیک نقطه آخر (هنگام حرکت ماوس)
    void updateActiveSegment(const QPointF& mousePos);

    // توابع الزامی QGraphicsItem
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QPainterPath shape() const override;

private:
    QVector<QPointF> m_points;   // لیست تمام نقاط شکست سیم
    QPainterPath m_path;         // مسیر رسم شده خطوط سیم
    void updatePath();           // تابع کمکی برای بازسازی QPainterPath
};

#endif // WIREITEM_H