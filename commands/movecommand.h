#ifndef MOVECOMMAND_H
#define MOVECOMMAND_H

#include "commandbase.h"
#include <QPointF>
#include <QMap>

class QGraphicsItem;

class MoveCommand : public CommandBase
{
public:
    MoveCommand(CircuitScene *scene,
                const QList<QGraphicsItem*> &items);

    void undo() override;
    void redo() override;

    void setNewPositions(const QMap<QGraphicsItem*, QPointF> &newPos);

private:
    QMap<QGraphicsItem*, QPointF> m_oldPos;
    QMap<QGraphicsItem*, QPointF> m_newPos;

    bool m_firstRedo = true;
};

#endif