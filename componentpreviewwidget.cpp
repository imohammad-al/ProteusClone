#include "componentpreviewwidget.h"
#include "component.h"
#include "componentfactory.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QtGlobal>

ComponentPreviewWidget::ComponentPreviewWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(140, 140);
}

ComponentPreviewWidget::~ComponentPreviewWidget()
{
    delete m_previewInstance;
}

void ComponentPreviewWidget::setComponentType(const QString &typeName)
{
    if (m_typeName == typeName && (m_previewInstance || typeName.isEmpty()))
        return;

    delete m_previewInstance;
    m_previewInstance = nullptr;
    m_typeName = typeName;

    if (!typeName.isEmpty()) {
        QGraphicsItem *item = ComponentFactory::create(typeName);
        m_previewInstance = dynamic_cast<Component *>(item);
        if (!m_previewInstance)
            delete item; // نوع ناشناخته یا کست ناموفق - چیزی برای پیش‌نمایش نیست
    }
    update();
}

void ComponentPreviewWidget::clear()
{
    setComponentType(QString());
}

void ComponentPreviewWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // پس‌زمینه خاکستری‌مایل‌به‌سبز، دقیقاً شبیه کادر Preview نرم‌افزار واقعی پروتئوس
    painter.fillRect(rect(), QColor(238, 238, 228));
    painter.setPen(QColor(190, 190, 180));
    painter.drawRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5));

    if (!m_previewInstance) {
        painter.setPen(QColor(140, 140, 140));
        painter.drawText(rect(), Qt::AlignCenter, tr("No Preview"));
        return;
    }

    const QRectF itemBounds = m_previewInstance->boundingRect();
    if (itemBounds.width() <= 0 || itemBounds.height() <= 0)
        return;

    const qreal margin = 14.0;
    const qreal availableW = qMax(1.0, width() - 2 * margin);
    const qreal availableH = qMax(1.0, height() - 2 * margin);
    const qreal scale = qMin(availableW / itemBounds.width(), availableH / itemBounds.height());

    painter.translate(width() / 2.0, height() / 2.0);
    painter.scale(scale, scale);
    painter.translate(-itemBounds.center());

    QStyleOptionGraphicsItem option;
    m_previewInstance->paint(&painter, &option, this);
}
