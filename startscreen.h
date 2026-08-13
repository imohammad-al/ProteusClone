#ifndef STARTSCREEN_H
#define STARTSCREEN_H

#include <QDialog>
#include <QString>
#include <QPoint>

class QStackedWidget;
class QListWidget;
class QListWidgetItem;
class QLabel;

// صفحه شروع نرم‌افزار (جایگزین StartupDialog قدیمی) - پیش از باز شدن پنجره
// اصلی نمایش داده می‌شود. شامل نوار کناری (Start/About)، سه کنش اصلی روی
// صفحه Start (New Project/Open Project/Template) و فهرست پروژه‌های اخیر است.
//
// بعد از exec() == QDialog::Accepted، در همه حالت‌ها (پروژه جدید ساخته‌شده،
// پروژه موجود باز‌شده، یا انتخاب از فهرست اخیر) chosenFilePath() یک مسیر
// معتبر روی دیسک برمی‌گرداند؛ main.cpp فقط کافی است آن را با
// MainWindow::openProjectFile() باز کند - چون پروژه‌ی تازه‌ساخته‌شده توسط
// NewProjectWizard هم از قبل واقعاً روی دیسک ذخیره شده است.
class StartScreen : public QDialog
{
    Q_OBJECT

public:
    explicit StartScreen(QWidget *parent = nullptr);

    QString chosenFilePath() const { return m_chosenFilePath; }

private slots:
    void onNewProjectClicked();
    void onOpenProjectClicked();
    void onTemplatesClicked();
    void onRecentItemActivated(QListWidgetItem *item);
    void onRecentContextMenuRequested(const QPoint &pos);
    void showStartPage();
    void showAboutPage();

private:
    void buildUi();
    QWidget *buildSidebar();
    QWidget *buildStartPage();
    QWidget *buildAboutPage();
    void reloadRecentProjects();
    void runNewProjectWizard(bool preferSchematic);
    void finishWith(const QString &filePath);

    QStackedWidget *m_stack = nullptr;
    QListWidget *m_recentList = nullptr;
    QLabel *m_emptyStateLabel = nullptr;

    QString m_chosenFilePath;
};

#endif // STARTSCREEN_H
