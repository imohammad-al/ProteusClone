#ifndef DELETEWIRECOMMAND_H
#define DELETEWIRECOMMAND_H

#include "commandbase.h"

class Wire;

// حذف/بازگردانی یک سیم. برخلاف DeleteComponentCommand که فقط addItem/removeItem
// عمومی روی صحنه گرافیکی انجام می‌دهد، این کلاس باید توپولوژی الکتریکی سیم
// (Node/Net/Junction) را هم از طریق CircuitScene::disconnectWire/reattachWire
// به‌درستی بشکند/بازسازی کند - چون سیم فقط یک آیتم گرافیکی مستقل نیست، بخشی
// از شبکه اتصالات مدار هم هست (بخش ۵.۵ مستند پروژه).
class DeleteWireCommand : public CommandBase
{
public:
    DeleteWireCommand(CircuitScene *scene, Wire *wire);
    ~DeleteWireCommand() override;

    void undo() override;
    void redo() override;

private:
    Wire *m_wire;
    bool m_inScene;
};

#endif // DELETEWIRECOMMAND_H
