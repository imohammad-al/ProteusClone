#ifndef DELETECOMPONENTCOMMAND_H
#define DELETECOMPONENTCOMMAND_H

#include "commandbase.h"
#include <QList>

class QGraphicsItem;
class Wire;

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

    // اگر m_item یک Component با پایه‌های سیم‌کشی‌شده باشد، سیم‌های متصل به آن باید
    // هنگام حذف قطعه هم از توپولوژی مدار جدا شوند و هنگام Undo دوباره وصل شوند -
    // در غیر این صورت بعد از پنهان‌شدن قطعه، آن سیم‌ها به‌صورت یتیم/معلق در صحنه می‌مانند.
    QList<Wire*> m_attachedWires;
};

#endif