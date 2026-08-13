#include "mainwindow.h"
#include "startscreen.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // برای این‌که QSettings پیش‌فرض (استفاده‌شده در RecentProjectsManager -
    // فهرست «پروژه‌های اخیر» صفحه Start) در یک مسیر مشخص و پایدار ذخیره کند.
    QApplication::setOrganizationName(QStringLiteral("ProteusClone"));
    QApplication::setApplicationName(QStringLiteral("ProteusClone"));

    StartScreen startScreen;
    if (startScreen.exec() != QDialog::Accepted)
        return 0; // کاربر صفحه شروع را بست/لغو کرد

    // در همه‌ی مسیرهای StartScreen (پروژه جدید، باز کردن پروژه، انتخاب از
    // فهرست اخیر یا یک قالب) chosenFilePath() یک فایل .pcproj معتبر روی دیسک
    // است - چون NewProjectWizard پروژه‌ی تازه را همان لحظه‌ی ساخت ذخیره می‌کند.
    MainWindow w;
    w.openProjectFile(startScreen.chosenFilePath());
    w.show();

    return QApplication::exec();
}
