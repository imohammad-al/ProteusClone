#include "logofactory.h"

#include <QPainter>
#include <QPen>

namespace {
// همان رنگ‌های IconFactory (kInk/kAccent) تا لوگو دقیقاً هم‌رنگ بقیه آیکون‌های
// برنامه باشد؛ چون آن ثابت‌ها private داخل iconfactory.cpp هستند، اینجا با
// همان مقادیر دوباره تعریف شده‌اند.
const QColor kInk(55, 55, 60);
const QColor kAccent(40, 110, 200);
const QColor kChipFill(246, 248, 251);
}

void LogoFactory::paint(QPainter &p, qreal size)
{
    p.setRenderHint(QPainter::Antialiasing);
    p.scale(size / 24.0, size / 24.0); // رسم روی یک بوم مرجع ۲۴×۲۴ (مثل IconFactory) و مقیاس‌دهی به اندازه واقعی

    // --- بدنه چیپ ---
    p.setPen(QPen(kInk, 1.6));
    p.setBrush(kChipFill);
    p.drawRoundedRect(QRectF(5, 5, 14, 14), 2.4, 2.4);

    // --- پایه‌ها (۳ تا از هر طرف، مثل یک آی‌سی واقعی روی شماتیک) ---
    p.setPen(QPen(kInk, 1.3));
    const qreal sidePositions[3] = {8, 12, 16};
    for (qreal y : sidePositions) {
        p.drawLine(QPointF(2, y), QPointF(5, y));
        p.drawLine(QPointF(19, y), QPointF(22, y));
    }
    for (qreal x : sidePositions) {
        p.drawLine(QPointF(x, 2), QPointF(x, 5));
        p.drawLine(QPointF(x, 19), QPointF(x, 22));
    }

    // --- نقطه «پایه شماره ۱» (قرارداد استاندارد نمادهای آی‌سی) با رنگ تاکیدی ---
    p.setPen(Qt::NoPen);
    p.setBrush(kAccent);
    p.drawEllipse(QPointF(7.6, 7.6), 1.15, 1.15);
}

QPixmap LogoFactory::pixmap(int size)
{
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    paint(p, size);
    return pm;
}

QIcon LogoFactory::icon()
{
    QIcon ic;
    for (int s : {16, 24, 32, 48, 64, 128})
        ic.addPixmap(pixmap(s));
    return ic;
}
