#include "pin.h"
#include "wire.h"
#include "node.h"
#include <QPainter>

Pin::Pin(QGraphicsItem *parent)
    : QGraphicsObject(parent)
{
    m_id = QUuid::createUuid();
    setFlag(QGraphicsItem::ItemSendsGeometryChanges); // برای مطلع شدن از جابجایی پین
}

QRectF Pin::boundingRect() const {
    return QRectF(-5, -5, 10, 10); // یک مربع کوچک برای محدوده کلیک پین
}

void Pin::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setPen(QPen(Qt::darkCyan, 1));
    painter->setBrush(Qt::cyan);
    painter->drawEllipse(QRectF(-3, -3, 6, 6)); // رسم پین به صورت دایره کوچک توپر شبیه پروتئوس
}

void Pin::addWire(Wire *wire) {
    if (wire && !m_wires.contains(wire)) {
        m_wires.append(wire);
    }
}

void Pin::removeWire(Wire *wire) {
    if (wire) {
        m_wires.removeOne(wire);
    }
}

void Pin::setNode(Node *node) {
    m_node = node;
}

void Pin::updateWires() {
    // هر زمان قطعه حرکت کند، این تابع صدا زده می‌شود و به تمام سیم‌های متصل دستور بازرسم می‌دهد
    for (Wire *wire : m_wires) {
        if (wire) {
            wire->rebuildGeometry();
        }
    }
}