#include "mirrorcommand.h"
#include "../component.h"

#include <QGraphicsItem>
#include <QTransform>

MirrorCommand::MirrorCommand(CircuitScene *scene, QGraphicsItem *item)
    : CommandBase(scene), m_item(item)
{
    setText("Mirror Component");
}

void MirrorCommand::toggleMirror()
{
    if (!m_item)
        return;

    QTransform t = m_item->transform();
    t.scale(-1, 1); // معکوس افقی حول محور y؛ اعمال دوباره همین عملیات آن را خنثی می‌کند
    m_item->setTransform(t);

    // مهم: مثل RotateCommand، بعد از تغییر هندسه باید سیم‌های متصل هم به‌روزرسانی شوند
    if (Component *c = dynamic_cast<Component*>(m_item))
        c->updateConnections();
}

void MirrorCommand::redo() { toggleMirror(); }
void MirrorCommand::undo() { toggleMirror(); }
