#include "wirecontroller.h"
#include "circuitscene.h"

WireController::WireController(CircuitScene* scene)
    : QObject(nullptr), m_scene(scene) // والد QObject را برابر nullptr قرار می‌دهیم
{
    // باقی کدهای سازنده در اینجا...
}

bool WireController::drawing() const
{
    return m_drawing;
}

void WireController::mousePress(const QPointF &, Pin *)
{
}

void WireController::mouseMove(const QPointF &)
{
}

void WireController::cancel()
{
}

void WireController::beginWire(Pin *)
{
}

void WireController::finishWire(Pin *)
{
}

void WireController::addCorner(const QPointF &)
{
}