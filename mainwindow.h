#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include "circuitscene.h"
#include "resistor.h"
#include <QKeyEvent>
#include <QTreeWidgetItem>
#include "toolmanager.h"
#include <QUndoStack>
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
protected:
    void keyPressEvent(QKeyEvent *event) override;
private:
    Ui::MainWindow *ui;

    CircuitScene *scene;

    void initializeComponentTree();

    ToolManager *toolManager;

    QUndoStack *undoStack;

private slots:

    void on_actionSelect_triggered();

    void on_actionWire_triggered();

    void on_actionResistor_triggered();

    void on_treeWidgetComponents_itemDoubleClicked(
        QTreeWidgetItem *item,
        int column);
};
#endif // MAINWINDOW_H
