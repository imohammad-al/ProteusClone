#ifndef MIRRORCOMMAND_H
#define MIRRORCOMMAND_H

#include "commandbase.h"

class QGraphicsItem;

// آینه‌کردن افقی یک قطعه با تغییر transform آن (مستقل از rotation موجود قطعه؛
// Qt این دو را جدا نگه می‌دارد و در نمایش نهایی ترکیب می‌کند - بخش ۴.۴ مستند پروژه).
// چون Mirror افقی یک عملیات خودمعکوس (involution) است، یک بار دیگر اجرایش دقیقاً
// حالت قبل را برمی‌گرداند؛ به همین دلیل undo() و redo() هر دو یک تابع مشترک را صدا می‌زنند.
class MirrorCommand : public CommandBase
{
public:
    MirrorCommand(CircuitScene *scene, QGraphicsItem *item);

    void undo() override;
    void redo() override;

private:
    void toggleMirror();

    QGraphicsItem *m_item;
};

#endif // MIRRORCOMMAND_H
