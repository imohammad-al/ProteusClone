#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "librarymanager.h"
#include "circuitscene.h"
#include "componentfactory.h"
#include "resistor.h"
#include <QPainter>
#include <QKeyEvent>
#include <QTreeWidgetItem>
#include <QMap>
#include "commands/deletecomponentcommand.h"
#include "commands/rotatecommand.h"




MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //-----------------------------------------
    // Component Library
    //-----------------------------------------

    initializeComponentTree();
    LibraryManager::initialize();

    toolManager = new ToolManager(this);

    undoStack=new QUndoStack(this);

    for(const ComponentInfo &c : LibraryManager::components())
    {
        ComponentFactory::registerComponent(
            c.name,
            c.creator);
    }
    //-----------------------------------------
    // Scene
    //-----------------------------------------

    scene = new CircuitScene(this);

    scene->setToolManager(toolManager);

    scene->setUndoStack(undoStack);

    scene->setSceneRect(0,0,3000,3000);

    ui->mainToolBar->addAction(ui->actionSelect);
    ui->mainToolBar->addAction(ui->actionWire);
    ui->mainToolBar->addAction(ui->actionResistor);
    ui->mainToolBar->addAction(
        undoStack->createUndoAction(this,"Undo"));

    ui->mainToolBar->addAction(
        undoStack->createRedoAction(this,"Redo"));

    ui->graphicsView->setScene(scene);

    ui->graphicsView->setViewportUpdateMode(
        QGraphicsView::FullViewportUpdate);

    ui->graphicsView->setRenderHint(
        QPainter::Antialiasing);

    ui->graphicsView->setDragMode(
        QGraphicsView::RubberBandDrag);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//////////////////////////////////////////////////////////
// Component Library
//////////////////////////////////////////////////////////
void MainWindow::initializeComponentTree()
{
    ui->treeWidgetComponents->clear();

    QMap<QString,QTreeWidgetItem*> categories;

    const QList<ComponentInfo> list =
        LibraryManager::components();

    for(const ComponentInfo &info : list)
    {
        QTreeWidgetItem *categoryItem = nullptr;

        if(categories.contains(info.category))
        {
            categoryItem = categories[info.category];
        }
        else
        {
            categoryItem =
                new QTreeWidgetItem(ui->treeWidgetComponents);

            categoryItem->setText(0, info.category);

            categories.insert(info.category, categoryItem);
        }

        QTreeWidgetItem *component =
            new QTreeWidgetItem(categoryItem);

        component->setText(0, info.name);

        component->setToolTip(0, info.description);
    }

    ui->treeWidgetComponents->expandAll();
}
//////////////////////////////////////////////////////////
// Double Click Component
//////////////////////////////////////////////////////////

void MainWindow::on_treeWidgetComponents_itemDoubleClicked(
    QTreeWidgetItem *item,
    int)
{
    // اگر روی یک گروه (Passive، Sources و...) کلیک شده
    if(item->childCount() > 0)
        return;
    toolManager->setCurrentTool(
        Tool::PlaceComponent);
    qDebug() << "Tool = PlaceComponent";

    toolManager->setComponentName(
        item->text(0));
    scene->beginComponentPlacement(
        item->text(0));
}

//////////////////////////////////////////////////////////
// Keyboard
//////////////////////////////////////////////////////////


void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(!scene)
    {
        QMainWindow::keyPressEvent(event);
        return;
    }

    //--------------------------------------
    // DELETE → Undo/Redo Command
    //--------------------------------------
    if(event->key() == Qt::Key_Delete)
    {
        QList<QGraphicsItem*> items =
            scene->selectedItems();

        for(QGraphicsItem *item : items)
        {
            if(undoStack)
            {
                undoStack->push(
                    new DeleteComponentCommand(
                        scene,
                        item));
            }
            else
            {
                delete item;
            }
        }

        return;
    }
    if(event->key() == Qt::Key_Escape)
    {
        toolManager->setCurrentTool(Tool::Select);

        scene->cancelPlacement();

        return;
    }
    //--------------------------------------
    // ROTATE → Undo/Redo Command
    //--------------------------------------
    if(event->key() == Qt::Key_R)
    {
        QList<QGraphicsItem*> items =
            scene->selectedItems();

        for(QGraphicsItem *item : items)
        {
            if(undoStack)
            {
                undoStack->push(
                    new RotateCommand(
                        scene,
                        item));
            }
            else
            {
                item->setRotation(
                    item->rotation() + 90);
            }
        }

        return;
    }

    QMainWindow::keyPressEvent(event);
}
//////////////////////////////////////////////////////////
// Toolbar
//////////////////////////////////////////////////////////

void MainWindow::on_actionSelect_triggered()
{
    toolManager->setCurrentTool(
        Tool::Select);

    scene->cancelPlacement();
    qDebug() << "Select Tool";
}
void MainWindow::on_actionWire_triggered()
{
    qDebug() << "Wire button clicked";

    toolManager->setCurrentTool(Tool::Wire);

    qDebug() << "Tool after click =" << int(toolManager->currentTool());

    scene->cancelPlacement();
}
void MainWindow::on_actionResistor_triggered()
{
    toolManager->setCurrentTool(
        Tool::PlaceComponent);

    toolManager->setComponentName(
        "Resistor");

    scene->beginComponentPlacement(
        "Resistor");
}