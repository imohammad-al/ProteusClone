#ifndef RECENTPROJECTSMANAGER_H
#define RECENTPROJECTSMANAGER_H

#include <QStringList>

// مدیریت فهرست «پروژه‌های اخیر» که در صفحه Start نمایش داده می‌شود، با
// QSettings ذخیره می‌شود - مستقل از هر پنجره خاصی تا هم StartScreen (برای
// نمایش) و هم MainWindow (هر بار که پروژه‌ای Open/Save As می‌شود) به آن
// دسترسی داشته باشند، بدون نیاز به رد و بدل کردن دستی این اطلاعات بین
// پنجره‌ها.
//
// نکته: main.cpp باید پیش از هر استفاده‌ای از QSettings، نام سازمان/برنامه را
// تنظیم کرده باشد (QApplication::setOrganizationName/setApplicationName) وگرنه
// QSettings پیش‌فرض در مسیر نامشخصی ذخیره می‌کند.
class RecentProjectsManager
{
public:
    // فهرست مسیرهایی که هنوز واقعاً روی دیسک وجود دارند (جدیدترین اول).
    // فایل‌های حذف‌شده به‌طور خودکار از خروجی (نه از حافظه ذخیره‌شده) فیلتر می‌شوند
    // تا اگر کاربر بعداً دوباره در همان مسیر فایلی بسازد، به‌جای گمشدن دائمی برگردد.
    static QStringList recentProjects(int maxCount = 10);

    // یک مسیر را به ابتدای فهرست اضافه می‌کند (اگر تکراری باشد فقط به ابتدا منتقل می‌شود).
    static void addRecentProject(const QString &filePath);

    // یک مسیر را صریحاً از فهرست حذف می‌کند (مثلاً از منوی راست‌کلیک در StartScreen) - خودِ فایل دست‌نخورده می‌ماند.
    static void removeRecentProject(const QString &filePath);

private:
    RecentProjectsManager() = delete; // این کلاس فقط شامل توابع استاتیک است

    static const char *kSettingsKey;
    static const int kMaxStored;
};

#endif // RECENTPROJECTSMANAGER_H
