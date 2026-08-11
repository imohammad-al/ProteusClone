#ifndef ICONFACTORY_H
#define ICONFACTORY_H

#include <QIcon>

// این پروژه از هیچ فایل تصویری خارجی (png/svg) استفاده نمی‌کند - طبق مستند پروژه
// پیاده‌سازی گرافیکی باید صرفاً با ++C/#C و کتابخانه‌های گرافیکی مبتنی بر آن‌ها
// باشد، و مسیر ساده‌تر/بی‌خطرتر (بدون نیاز به pipeline دارایی‌های تصویری و فایل
// .qrc) این است که هر آیکون را مستقیماً با QPainter روی یک QPixmap کوچک رسم
// کنیم. سبک بصری عمداً ساده و تک‌رنگ (خطی) نگه داشته شده، شبیه آیکون‌های واقعی
// Proteus (بخش «تکمیل ظاهری» - درخواست کاربر بعد از فاز ۱۶).
class IconFactory
{
public:
    // --- File ---
    static QIcon newProjectIcon();
    static QIcon openProjectIcon();
    static QIcon saveProjectIcon();
    static QIcon saveAsIcon();
    static QIcon exportImageIcon();

    // --- Edit ---
    static QIcon undoIcon();
    static QIcon redoIcon();

    // --- Modes (نوار کناری، شبیه Sidebar واقعی پروتئوس) ---
    static QIcon selectIcon();
    static QIcon wireIcon();
    static QIcon placeComponentIcon();
    static QIcon rotateCwIcon();
    static QIcon rotateCcwIcon();
    static QIcon mirrorIcon();

    // --- Tools / Simulation ---
    static QIcon checkDesignIcon();
    static QIcon pickPartsIcon();
    static QIcon runIcon();
    static QIcon pauseIcon();
    static QIcon stopIcon();
    static QIcon stepIcon();

    // --- View ---
    static QIcon zoomInIcon();
    static QIcon zoomOutIcon();
    static QIcon zoomResetIcon();

private:
    // یک بوم خالی ۲۴×۲۴ با پس‌زمینه شفاف و قلم‌موی استاندارد آماده می‌کند تا هر
    // تابع icon فقط شکل خودش را رویش رسم کند - از تکرار کد جلوگیری می‌کند.
    static QPixmap blankCanvas(int size = 24);
};

#endif // ICONFACTORY_H
