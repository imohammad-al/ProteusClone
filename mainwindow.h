#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include "circuitscene.h"
#include <QKeyEvent>
#include <QListWidgetItem>
#include <QSize>
#include "toolmanager.h"
#include <QUndoStack>

class SimulationEngine;
enum class SimulationState;
class QLabel;

QT_BEGIN_NAMESPACE

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    // برای اتصال به صفحه شروع برنامه (بخش ۱ مستند پروژه) - main.cpp این دو تابع را
    // بر اساس انتخاب کاربر در StartScreen صدا می‌زند. توجه: در جریان فعلی
    // (نگاه کن main.cpp)، چون StartScreen/NewProjectWizard همیشه یک فایل واقعی
    // روی دیسک برمی‌گردانند، فقط openProjectFile() صدا زده می‌شود؛ setCanvasSize()
    // برای استفاده‌های آینده (مثلاً یک منوی Page Setup برای تغییر اندازه بومِ
    // یک پروژه‌ی از قبل بازشده) نگه داشته شده است.
    void openProjectFile(const QString &path);
    void setCanvasSize(const QSize &size);
protected:
    void keyPressEvent(QKeyEvent *event) override;
private:
    Ui::MainWindow *ui;

    CircuitScene *scene;

    // قطعه را (اگر از قبل نبود) به لیست کناری «قطعات فعال» اضافه می‌کند (بخش ۳.۴ مستند).
    void addComponentToActiveList(const QString &componentName);

    // قطعه‌ی داده‌شده را برای قرارگیری روی بوم با کلیک بعدی آماده می‌کند.
    void armComponentForPlacement(const QString &componentName);

    ToolManager *toolManager;

    QUndoStack *undoStack;

    SimulationEngine *simEngine;

    QLabel *m_coordinatesLabel = nullptr; // نمایش زنده مختصات ماوس روی بوم (بخش ۲ مستند)

    QString m_lastSelectedComponent = "Resistor"; // قطعه پیش‌فرض

    QString m_currentFilePath; // مسیر فایل پروژه فعلی؛ خالی یعنی هنوز ذخیره نشده (بخش ۱۰.۱ مستند)

    void openComponentSelectionDialog();          // پنجره Pick Devices (بخش ۳.۱-۳.۳ مستند) را باز می‌کند

    void updateWindowTitle();                      // نام فایل فعلی را در عنوان پنجره نشان می‌دهد

private slots:

    void on_actionSelect_triggered();

    void on_actionWire_triggered();

    void on_actionResistor_triggered();

    void on_actionPickParts_triggered();

    void on_pushButtonPickParts_clicked();

    void on_pushButtonRemovePart_clicked();

    void on_listWidgetActiveParts_itemClicked(QListWidgetItem *item);

    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionExportImage_triggered();
    void on_actionCheckDesign_triggered();

    void on_actionRun_triggered();
    void on_actionPause_triggered();
    void on_actionStop_triggered();
    void on_actionStep_triggered();

    void on_actionRotateCW_triggered();
    void on_actionRotateCCW_triggered();
    void on_actionMirror_triggered();

    void onSimulationStateChanged(SimulationState state);
};
#endif // MAINWINDOW_H
