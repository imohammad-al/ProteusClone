#ifndef ROTATECOMMAND_H
#define ROTATECOMMAND_H

#include "commandbase.h"

class QGraphicsItem;

class RotateCommand : public CommandBase
{
public:
    RotateCommand(CircuitScene *scene,
                  QGraphicsItem *item,
                  qreal angle = 90);

    void undo() override;
    void redo() override;

private:
    QGraphicsItem *m_item;

    qreal m_oldRotation;
    qreal m_newRotation;
};

#endif