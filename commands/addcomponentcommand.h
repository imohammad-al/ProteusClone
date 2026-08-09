#ifndef ADDCOMPONENTCOMMAND_H
#define ADDCOMPONENTCOMMAND_H

#include "commandbase.h"

class QGraphicsItem;

class AddComponentCommand : public CommandBase
{
public:
    AddComponentCommand(
        CircuitScene *scene,
        QGraphicsItem *item);

    ~AddComponentCommand() override;

    void undo() override;
    void redo() override;

private:
    QGraphicsItem *m_item;

    bool m_inScene;
};

#endif