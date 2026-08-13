#ifndef LOGOFACTORY_H
#define LOGOFACTORY_H

#include <QIcon>
#include <QPixmap>

// لوگوی ProteusClone.
//
// دقیقاً هم‌سو با تصمیم قبلی خود پروژه در IconFactory (نگاه کن iconfactory.h:
// «این پروژه از هیچ فایل تصویری خارجی (png/svg) استفاده نمی‌کند»)، لوگو هم
// مستقیماً با QPainter رسم می‌شود - نه از روی یک فایل روی دیسک. این یعنی:
//   ۱) هیچ فایلی نیاز به قرار گرفتن در هیچ مسیری ندارد و هیچ‌وقت «گم» نمی‌شود.
//   ۲) در هر اندازه‌ای (۱۶ تا ۱۲۸ پیکسل) کاملاً واضح و بدون بلور می‌ماند، چون
//      هر بار از نو و متناسب با اندازه درخواستی رسم می‌شود (نه Scale یک بیت‌مپ).
//   ۳) نیازی به ماژول Qt6::Svg یا فایل .qrc اضافه در CMakeLists.txt نیست.
//
// طرح لوگو یک آی‌سی شماتیک (چیپ با پایه از هر ۴ طرف + نقطه پایه شماره ۱ با
// رنگ تاکیدی) است - همان زبان بصری‌ای که IconFactory برای بقیه آیکون‌های
// برنامه استفاده کرده (رنگ‌های kInk/kAccent مشترک).
class LogoFactory
{
public:
    // لوگو را در اندازه دلخواه (مربعی، پس‌زمینه شفاف) برمی‌گرداند.
    static QPixmap pixmap(int size);

    // یک QIcon چند-رزولوشنی (برای windowIcon پنجره‌ها) بر پایه pixmap() می‌سازد.
    static QIcon icon();

private:
    static void paint(QPainter &painter, qreal size);
};

#endif // LOGOFACTORY_H
