#ifndef DELETECOMPONENTCOMMAND_H
#define DELETECOMPONENTCOMMAND_H

#include "commandbase.h"

class QGraphicsItem;

class DeleteComponentCommand : public CommandBase
{
public:
    DeleteComponentCommand(
        CircuitScene *scene,
        QGraphicsItem *item);

    ~DeleteComponentCommand() override;

    void undo() override;
    void redo() override;

private:
    QGraphicsItem *m_item;

    bool m_inScene;
};

#endif