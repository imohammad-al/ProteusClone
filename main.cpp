#include "mainwindow.h"
#include "startupdialog.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    StartupDialog startup;
    if (startup.exec() != QDialog::Accepted)
        return 0; // کاربر دیالوگ شروع را بست/لغو کرد

    MainWindow w;

    if (startup.choice() == StartupDialog::Choice::OpenExisting)
        w.openProjectFile(startup.chosenFilePath());
    else
        w.setCanvasSize(startup.chosenCanvasSize());

    w.show();
    return QApplication::exec();
}
