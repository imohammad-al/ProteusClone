#ifndef STARTUPDIALOG_H
#define STARTUPDIALOG_H

#include <QDialog>
#include <QSize>
#include <QString>

class QSpinBox;

// دیالوگی که پیش از باز شدن پنجره اصلی نمایش داده می‌شود (بخش ۱ مستند پروژه):
// کاربر یا یک پروژه جدید با اندازه بوم دلخواه می‌سازد، یا یک پروژه موجود را
// از دیسک باز می‌کند. main.cpp بر اساس choice()/chosenCanvasSize()/chosenFilePath()
// تصمیم می‌گیرد MainWindow را چطور مقداردهی اولیه کند.
//
// نکته: عمداً از نام result() برای عضو تازه استفاده نشده چون QDialog از قبل یک
// result() با معنای دیگر (Accepted/Rejected) دارد و هم‌نام کردنشان گمراه‌کننده است.
class StartupDialog : public QDialog
{
    Q_OBJECT

public:
    enum class Choice {
        NewProject,
        OpenExisting,
        Cancelled
    };

    explicit StartupDialog(QWidget *parent = nullptr);

    Choice choice() const { return m_choice; }
    QSize chosenCanvasSize() const;
    QString chosenFilePath() const { return m_filePath; }

private slots:
    void onNewProjectClicked();
    void onOpenExistingClicked();

private:
    Choice m_choice = Choice::Cancelled;
    QString m_filePath;

    QSpinBox *m_widthSpin = nullptr;
    QSpinBox *m_heightSpin = nullptr;
};

#endif // STARTUPDIALOG_H
