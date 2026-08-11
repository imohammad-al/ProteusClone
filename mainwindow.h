#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include "circuitscene.h"
#include <QKeyEvent>
#include <QTreeWidgetItem>
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

    // برای اتصال به دیالوگ شروع برنامه (بخش ۱ مستند پروژه) - main.cpp این دو تابع را
    // بر اساس انتخاب کاربر در StartupDialog صدا می‌زند
    void openProjectFile(const QString &path);
    void setCanvasSize(const QSize &size);
protected:
    void keyPressEvent(QKeyEvent *event) override;
private:
    Ui::MainWindow *ui;

    CircuitScene *scene;

    void initializeComponentTree();

    ToolManager *toolManager;

    QUndoStack *undoStack;

    SimulationEngine *simEngine;

    QLabel *m_coordinatesLabel = nullptr; // نمایش زنده مختصات ماوس روی بوم (بخش ۲ مستند)

    QString m_lastSelectedComponent = "Resistor"; // قطعه پیش‌فرض

    QString m_currentFilePath; // مسیر فایل پروژه فعلی؛ خالی یعنی هنوز ذخیره نشده (بخش ۱۰.۱ مستند)

    void openComponentSelectionDialog();          // این تابع پنجره‌ی لیست قطعات را باز خواهد کرد

    void updateWindowTitle();                      // نام فایل فعلی را در عنوان پنجره نشان می‌دهد

private slots:

    void on_actionSelect_triggered();

    void on_actionWire_triggered();

    void on_actionResistor_triggered();

    void on_treeWidgetComponents_itemDoubleClicked(
        QTreeWidgetItem *item,
        int column);

    void on_lineEditSearch_textChanged(const QString &text);

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

    void onSimulationStateChanged(SimulationState state);
};
#endif // MAINWINDOW_H
