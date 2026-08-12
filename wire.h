#ifndef WIRE_H
#define WIRE_H

#include <QGraphicsPathItem>
#include <QPointF>
#include <QVector>

class Pin;

class Wire : public QGraphicsPathItem {
public:
    Wire(Pin* startPin, Pin* endPin, QGraphicsItem* parent = nullptr);
    ~Wire();

    Pin* startPin() const { return m_startPin; }
    Pin* endPin() const { return m_endPin; }

    // تابع کلیدی پروپوزال برای بازسازی هندسه سیم هنگام جابجایی قطعات
    void rebuildGeometry();

    // نقاط شکست مسیر سیم (شروع، نقطه واسط ۹۰ درجه، پایان) در فضای بوم - برای
    // محاسبه فاصله از یک نقطه دلخواه تا این سیم (اسنپ خودکار پین هنگام قرار دادن
    // قطعه نزدیک یک سیم - بخش ۲ درخواست کاربر).
    QVector<QPointF> points() const { return m_points; }

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    Pin* m_startPin;
    Pin* m_endPin;
    QVector<QPointF> m_points; // نقاط شکستگی سیم (برای رسم ۹۰ درجه چرخیده شبیه پروتئوس)
};

#endif // WIRE_H