#include "deletewirecommand.h"

#include "../circuitscene.h"
#include "../wire.h"

DeleteWireCommand::DeleteWireCommand(CircuitScene *scene, Wire *wire)
    : CommandBase(scene),
    m_wire(wire),
    m_inScene(true)
{
    setText("Delete Wire");
}

DeleteWireCommand::~DeleteWireCommand()
{
    if (!m_inScene && m_wire)
        delete m_wire;
}

void DeleteWireCommand::redo()
{
    if (!m_scene || !m_wire)
        return;

    if (m_inScene) {
        m_scene->disconnectWire(m_wire); // پین/گره را جدا و از صحنه حذف می‌کند (بدون delete شیء)
        m_inScene = false;
    }
}

void DeleteWireCommand::undo()
{
    if (!m_scene || !m_wire)
        return;

    if (!m_inScene) {
        m_scene->reattachWire(m_wire); // دوباره به صحنه و توپولوژی متصل می‌کند
        m_inScene = true;
    }
}
