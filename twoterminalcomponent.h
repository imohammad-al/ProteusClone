#ifndef TWOTERMINALCOMPONENT_H
#define TWOTERMINALCOMPONENT_H

#include "component.h"
#include "pin.h"

class TwoTerminalComponent : public Component
{
    Q_OBJECT

public:

    explicit TwoTerminalComponent(
        QGraphicsItem *parent=nullptr);

    Pin *leftPin() const;

    Pin *rightPin() const;

protected:

    Pin *m_leftPin;

    Pin *m_rightPin;
};

#endif