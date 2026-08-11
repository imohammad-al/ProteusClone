#include "iconfactory.h"

#include <QPainter>
#include <QPixmap>
#include <QPen>
#include <QPolygonF>
#include <QPainterPath>

namespace {
const QColor kInk(55, 55, 60);       // رنگ اصلی خطوط (تقریباً مشکی، نه کاملاً)
const QColor kAccent(40, 110, 200);  // رنگ تاکیدی آبی (برای Run/برجسته‌سازی)
const QColor kStop(190, 55, 45);     // قرمز برای Stop
const QColor kGo(35, 140, 60);       // سبز برای Run
}

QPixmap IconFactory::blankCanvas(int size)
{
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    return pm;
}

// ============================= File =============================

QIcon IconFactory::newProjectIcon()
{
    QPixmap pm = blankCanvas();
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(kInk, 1.6));
    p.setBrush(Qt::white);

    QPolygonF page;
    page << QPointF(6, 3) << QPointF(15, 3) << QPointF(19, 7)
         << QPointF(19, 21) << QPointF(6, 21);
    p.drawPolygon(page);
    p.drawLine(QPointF(15, 3), QPointF(15, 7));
    p.drawLine(QPointF(15, 7), QPointF(19, 7));

    p.setPen(QPen(kAccent, 1.8));
    p.drawLine(QPointF(9, 14), QPointF(15, 14));
    p.drawLine(QPointF(12, 11), QPointF(12, 17));
    return QIcon(pm);
}

QIcon IconFactory::openProjectIcon()
{
    QPixmap pm = blankCanvas();
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(kInk, 1.6));
    p.setBrush(QColor(255, 221, 130));

    QPolygonF backFlap;
    backFlap << QPointF(3, 7) << QPointF(9, 7) << QPointF(11, 9) << QPointF(21, 9)
             << QPointF(21, 18) << QPointF(3, 18);
    p.drawPolygon(backFlap);

    p.setBrush(QColor(255, 200, 90));
    QPolygonF frontFlap;
    frontFlap << QPointF(3, 18) << QPointF(5, 11) << QPointF(22, 11) << QPointF(20, 18);
    p.drawPolygon(frontFlap);
    return QIcon(pm);
}

QIcon IconFactory::saveProjectIcon()
{
    QPixmap pm = blankCanvas();
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(kInk, 1.6));
    p.setBrush(QColor(230, 230, 235));

    QPolygonF body;
    body << QPointF(4, 3) << QPointF(17, 3) << QPointF(21, 7)
         << QPointF(21, 21) << QPointF(4, 21);
    p.drawPolygon(body);

    p.setBrush(Qt::white);
    p.drawRect(QRectF(7, 3, 8, 6));
    p.setBrush(QColor(120, 170, 230));
    p.drawRect(QRectF(7, 13, 10, 7));
    return QIcon(pm);
}

QIcon IconFactory::saveAsIcon()
{
    QPixmap pm = saveProjectIcon().pixmap(24, 24);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(kAccent, 2.0));
    p.drawLine(QPointF(17, 16), QPointF(17, 22));
    p.drawLine(QPointF(14, 19), QPointF(20, 19));
    return QIcon(pm);
}

QIcon IconFactory::exportImageIcon()
{
    QPixmap pm = blankCanvas();
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(kInk, 1.6));
    p.setBrush(Qt::white);
    p.drawRect(QRectF(3, 4, 14, 12));

    p.setBrush(QColor(240, 190, 80));
    p.drawEllipse(QPointF(8, 9), 1.6, 1.6);
    QPolygonF mountains;
    mountains << QPointF(4, 15) << QPointF(9, 10) << QPointF(12, 13)
              << QPointF(14, 11) << QPointF(16, 15);
    p.setBrush(QColor(120, 170, 110));
    p.drawPolygon(mountains);

    p.setPen(QPen(kAccent, 1.8));
    p.drawLine(QPointF(18, 12), QPointF(22, 12));
    p.drawLine(QPointF(22, 12), QPointF(19, 9));
    p.drawLine(QPointF(22, 12), QPointF(19, 15));
    return QIcon(pm);
}

// ============================= Edit =============================

QIcon IconFactory::undoIcon()
{
    QPixmap pm = blankCanvas();
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(kInk, 2.0, Qt::SolidLine, Qt::RoundCap));
    p.setBrush(Qt::NoBrush);
    p.drawArc(QRectF(4, 5, 15, 15), 20 * 16, 250 * 16);

    p.setBrush(kInk);
    p.setPen(Qt::NoPen);
    QPolygonF arrowHead;
    arrowHead << QPointF(4, 6) << QPointF(9, 6) << QPointF(5.5, 11);
    p.drawPolygon(arrowHead);
    return QIcon(pm);
}

QIcon IconFactory::redoIcon()
{
    QImage img = undoIcon().pixmap(24, 24).toImage().mirrored(true, false);
    return QIcon(QPixmap::fromImage(img));
}

// ============================= Modes =============================

QIcon IconFactory::selectIcon()
{
    QPixmap pm = blankCanvas();
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(kInk, 1.4));
    p.setBrush(kInk);

    QPolygonF arrow;
    arrow << QPointF(5, 3) << QPointF(5, 19) << QPointF(9, 15)
          << QPointF(12, 21) << QPointF(14, 20) << QPointF(11, 14)
          << QPointF(17, 13);
    p.drawPolygon(arrow);
    return QIcon(pm);
}

QIcon IconFactory::wireIcon()
{
    QPixmap pm = blankCanvas();
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(kGo, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

    QPainterPath path;
    path.moveTo(3, 6);
    path.lineTo(10, 6);
    path.lineTo(10, 18);
    path.lineTo(21, 18);
    p.drawPath(path);

    p.setPen(QPen(kInk, 1.2));
    p.setBrush(Qt::white);
    p.drawEllipse(QPointF(3, 6), 2, 2);
    p.drawEllipse(QPointF(21, 18), 2, 2);
    return QIcon(pm);
}

QIcon IconFactory::placeComponentIcon()
{
    QPixmap pm = blankCanvas();
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(kInk, 1.8));

    p.drawLine(QPointF(2, 12), QPointF(6, 12));
    p.drawLine(QPointF(18, 12), QPointF(22, 12));
    p.setBrush(QColor(250, 235, 205));
    p.drawRect(QRectF(6, 8, 12, 8));
    return QIcon(pm);
}

QIcon IconFactory::rotateCwIcon()
{
    QPixmap pm = blankCanvas();
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(kInk, 2.0, Qt::SolidLine, Qt::RoundCap));
    p.setBrush(Qt::NoBrush);
    p.drawArc(QRectF(4, 4, 16, 16), 40 * 16, 260 * 16);

    p.setPen(Qt::NoPen);
    p.setBrush(kInk);
    QPolygonF arrowHead;
    arrowHead << QPointF(18, 4) << QPointF(21, 8.5) << QPointF(15.5, 8.7);
    p.drawPolygon(arrowHead);
    return QIcon(pm);
}

QIcon IconFactory::rotateCcwIcon()
{
    QImage img = rotateCwIcon().pixmap(24, 24).toImage().mirrored(true, false);
    return QIcon(QPixmap::fromImage(img));
}

QIcon IconFactory::mirrorIcon()
{
    QPixmap pm = blankCanvas();
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);

    p.setPen(QPen(kInk, 1.2, Qt::DashLine));
    p.drawLine(QPointF(12, 2), QPointF(12, 22));

    p.setPen(QPen(kInk, 1.4));
    p.setBrush(QColor(120, 170, 230));
    QPolygonF left;
    left << QPointF(10, 6) << QPointF(10, 18) << QPointF(3, 12);
    p.drawPolygon(left);

    QPolygonF right;
    right << QPointF(14, 6) << QPointF(14, 18) << QPointF(21, 12);
    p.drawPolygon(right);
    return QIcon(pm);
}

// ============================= Tools / Simulation =============================

QIcon IconFactory::pickPartsIcon()
{
    QPixmap pm = blankCanvas();
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);

    p.setPen(QPen(kInk, 1.6));
    p.setBrush(QColor(250, 235, 205));
    p.drawRect(QRectF(2, 9, 11, 7));
    p.drawLine(QPointF(4, 9), QPointF(4, 6));
    p.drawLine(QPointF(9, 9), QPointF(9, 6));
    p.drawLine(QPointF(4, 16), QPointF(4, 19));
    p.drawLine(QPointF(9, 16), QPointF(9, 19));

    p.setPen(QPen(kAccent, 1.8));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(QRectF(11, 3, 9, 9));
    p.drawLine(QPointF(17.5, 9.5), QPointF(21.5, 13.5));
    return QIcon(pm);
}

QIcon IconFactory::checkDesignIcon()
{
    QPixmap pm = blankCanvas();
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(kInk, 1.6));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(QRectF(2, 2, 14, 14));
    p.drawLine(QPointF(13, 13), QPointF(21, 21));

    p.setPen(QPen(kGo, 2.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    QPainterPath check;
    check.moveTo(5.5, 9);
    check.lineTo(8, 12);
    check.lineTo(12.5, 5.5);
    p.drawPath(check);
    return QIcon(pm);
}

QIcon IconFactory::runIcon()
{
    QPixmap pm = blankCanvas();
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(kGo.darker(120), 1.0));
    p.setBrush(kGo);
    QPolygonF triangle;
    triangle << QPointF(6, 4) << QPointF(6, 20) << QPointF(20, 12);
    p.drawPolygon(triangle);
    return QIcon(pm);
}

QIcon IconFactory::pauseIcon()
{
    QPixmap pm = blankCanvas();
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(210, 150, 40));
    p.drawRect(QRectF(6, 4, 4, 16));
    p.drawRect(QRectF(14, 4, 4, 16));
    return QIcon(pm);
}

QIcon IconFactory::stopIcon()
{
    QPixmap pm = blankCanvas();
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(kStop);
    p.drawRoundedRect(QRectF(5, 5, 14, 14), 2, 2);
    return QIcon(pm);
}

QIcon IconFactory::stepIcon()
{
    QPixmap pm = blankCanvas();
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(kGo);
    QPolygonF triangle;
    triangle << QPointF(4, 5) << QPointF(4, 19) << QPointF(15, 12);
    p.drawPolygon(triangle);
    p.drawRect(QRectF(17, 5, 3, 14));
    return QIcon(pm);
}

// ============================= View =============================

QIcon IconFactory::zoomInIcon()
{
    QPixmap pm = blankCanvas();
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(kInk, 1.8));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(QRectF(3, 3, 12, 12));
    p.drawLine(QPointF(13, 13), QPointF(20, 20));

    p.drawLine(QPointF(6, 9), QPointF(12, 9));
    p.drawLine(QPointF(9, 6), QPointF(9, 12));
    return QIcon(pm);
}

QIcon IconFactory::zoomOutIcon()
{
    QPixmap pm = blankCanvas();
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(kInk, 1.8));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(QRectF(3, 3, 12, 12));
    p.drawLine(QPointF(13, 13), QPointF(20, 20));

    p.drawLine(QPointF(6, 9), QPointF(12, 9));
    return QIcon(pm);
}

QIcon IconFactory::zoomResetIcon()
{
    QPixmap pm = blankCanvas();
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(kInk, 1.8, Qt::SolidLine, Qt::RoundCap));

    p.drawLine(QPointF(3, 8), QPointF(3, 3));
    p.drawLine(QPointF(3, 3), QPointF(8, 3));

    p.drawLine(QPointF(21, 8), QPointF(21, 3));
    p.drawLine(QPointF(21, 3), QPointF(16, 3));

    p.drawLine(QPointF(3, 16), QPointF(3, 21));
    p.drawLine(QPointF(3, 21), QPointF(8, 21));

    p.drawLine(QPointF(21, 16), QPointF(21, 21));
    p.drawLine(QPointF(21, 21), QPointF(16, 21));
    return QIcon(pm);
}
