#include "deletecomponentcommand.h"

#include "../circuitscene.h"
#include "../component.h"
#include "../pin.h"
#include "../wire.h"

#include <QGraphicsItem>

DeleteComponentCommand::DeleteComponentCommand(
    CircuitScene *scene,
    QGraphicsItem *item)
    : CommandBase(scene),
    m_item(item),
    m_inScene(true)
{
    setText("Delete Component");
}

DeleteComponentCommand::~DeleteComponentCommand()
{
    if(!m_inScene && m_item)
        delete m_item;
}

void DeleteComponentCommand::redo()
{
    if(!m_scene || !m_item)
        return;

    if(m_inScene)
    {
        // اگر این آیتم یک قطعه با پایه‌های سیم‌کشی‌شده است، اول سیم‌های متصل به آن را
        // از توپولوژی مدار جدا کن - وگرنه بعد از پنهان‌شدن قطعه، آن سیم‌ها به‌صورت
        // یتیم/معلق (بدون گره معتبر) در صحنه باقی می‌مانند.
        if (Component *comp = dynamic_cast<Component*>(m_item)) {
            m_attachedWires.clear();
            const QList<Pin*> pins = comp->pins();
            for (Pin *p : pins) {
                if (!p) continue;
                const QList<Wire*> pinWires = p->wires();
                for (Wire *w : pinWires) {
                    if (!m_attachedWires.contains(w))
                        m_attachedWires.append(w);
                }
            }
            for (Wire *w : m_attachedWires)
                m_scene->disconnectWire(w);
        }

        m_scene->removeItem(m_item);
        m_inScene = false;
    }
}

void DeleteComponentCommand::undo()
{
    if(!m_scene || !m_item)
        return;

    if(!m_inScene)
    {
        m_scene->addItem(m_item);
        m_inScene = true;

        // سیم‌هایی که قبل از حذف به این قطعه وصل بودند را دوباره متصل کن
        for (Wire *w : m_attachedWires)
            m_scene->reattachWire(w);
    }
}