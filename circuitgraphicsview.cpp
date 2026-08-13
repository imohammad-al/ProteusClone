#include "circuitgraphicsview.h"

#include <QWheelEvent>
#include <QKeyEvent>

CircuitGraphicsView::CircuitGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    setDragMode(QGraphicsView::RubberBandDrag);
    setMouseTracking(true); // برای دریافت mouseMoveEvent حتی وقتی هیچ دکمه‌ای فشرده نشده (نمایش زنده مختصات)
}

void CircuitGraphicsView::applyZoom(qreal factor)
{
    const qreal newZoom = m_zoomFactor * factor;
    if (newZoom < kMinZoom || newZoom > kMaxZoom)
        return;

    m_zoomFactor = newZoom;
    scale(factor, factor);
}

void CircuitGraphicsView::zoomIn() { applyZoom(1.15); }
void CircuitGraphicsView::zoomOut() { applyZoom(1.0 / 1.15); }

void CircuitGraphicsView::resetZoom()
{
    resetTransform();
    m_zoomFactor = 1.0;
}

void CircuitGraphicsView::wheelEvent(QWheelEvent *event)
{
    // بخش ۲ درخواست کاربر: چرخاندن ساده چرخ‌ماوس روی بوم (بدون نیاز به نگه‌داشتن
    // Ctrl) باید zoom in/out کند - دقیقاً مثل رفتار پیش‌فرض خودِ Proteus. مرکز زوم
    // زیر نشانگر ماوس می‌ماند چون AnchorUnderMouse در سازنده تنظیم شده است.
    const int delta = event->angleDelta().y();
    if (delta != 0) {
        const qreal factor = (delta > 0) ? 1.15 : (1.0 / 1.15);
        applyZoom(factor);
        event->accept();
        return;
    }

    QGraphicsView::wheelEvent(event);
}

void CircuitGraphicsView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
        m_dragModeBeforePan = dragMode();
        setDragMode(QGraphicsView::ScrollHandDrag);
        event->accept();
        return;
    }

    QGraphicsView::keyPressEvent(event);
}

void CircuitGraphicsView::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
        setDragMode(m_dragModeBeforePan);
        event->accept();
        return;
    }

    QGraphicsView::keyReleaseEvent(event);
}

void CircuitGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    emit mouseScenePositionChanged(mapToScene(event->pos()));
    QGraphicsView::mouseMoveEvent(event);
}
