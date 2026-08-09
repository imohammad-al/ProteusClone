#ifndef CIRCUITGRAPHICSVIEW_H
#define CIRCUITGRAPHICSVIEW_H

#include <QGraphicsView>

// این کلاس فقط مسئول تعامل‌های سطح View با بوم است که ربطی به منطق مدار ندارد
// (بخش ۲ مستند پروژه): زوم با Ctrl+چرخ‌ماوس، Pan با نگه‌داشتن Space، و اعلام
// موقعیت لحظه‌ای ماوس روی بوم برای نمایش در نوار وضعیت.
class CircuitGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit CircuitGraphicsView(QWidget *parent = nullptr);

public slots:
    void zoomIn();
    void zoomOut();
    void resetZoom();

signals:
    void mouseScenePositionChanged(QPointF scenePos);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void applyZoom(qreal factor);

    qreal m_zoomFactor = 1.0;
    QGraphicsView::DragMode m_dragModeBeforePan = QGraphicsView::RubberBandDrag;

    static constexpr qreal kMinZoom = 0.2;
    static constexpr qreal kMaxZoom = 5.0;
};

#endif // CIRCUITGRAPHICSVIEW_H
