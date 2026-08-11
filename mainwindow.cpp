#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "librarymanager.h"
#include "circuitscene.h"
#include "circuitgraphicsview.h"
#include "componentfactory.h"
#include "io/projectserializer.h"
#include "sim/designrulechecker.h"
#include "sim/simulationlogger.h"
#include "sim/simulationengine.h"

#include <QPainter>
#include <QKeyEvent>
#include <QTreeWidgetItem>
#include <QMap>
#include <QInputDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QLabel>
#include "commands/deletecomponentcommand.h"
#include "commands/deletewirecommand.h"
#include "commands/rotatecommand.h"
#include "commands/mirrorcommand.h"
#include "wire.h"




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

    simEngine = new SimulationEngine(scene, this);
    connect(simEngine, &SimulationEngine::stateChanged,
            this, &MainWindow::onSimulationStateChanged);
    onSimulationStateChanged(SimulationState::Stopped);

    ui->actionResistor->setText("Components");
    ui->mainToolBar->addAction(ui->actionSelect);
    ui->mainToolBar->addAction(ui->actionWire);
    ui->mainToolBar->addAction(ui->actionResistor);
    ui->mainToolBar->addAction(
        undoStack->createUndoAction(this,"Undo"));

    ui->mainToolBar->addAction(
        undoStack->createRedoAction(this,"Redo"));

    ui->graphicsView->setScene(scene);

    ui->graphicsView->setViewportUpdateMode(
        QGraphicsView::SmartViewportUpdate);

    ui->graphicsView->setRenderHint(
        QPainter::Antialiasing);

    ui->graphicsView->setDragMode(
        QGraphicsView::RubberBandDrag);

    //-----------------------------------------
    // Zoom / Pan / Coordinates (بخش ۲ مستند پروژه)
    //-----------------------------------------

    connect(ui->actionZoomIn, &QAction::triggered, ui->graphicsView, &CircuitGraphicsView::zoomIn);
    connect(ui->actionZoomOut, &QAction::triggered, ui->graphicsView, &CircuitGraphicsView::zoomOut);
    connect(ui->actionZoomReset, &QAction::triggered, ui->graphicsView, &CircuitGraphicsView::resetZoom);

    m_coordinatesLabel = new QLabel(tr("X: 0, Y: 0"), this);
    ui->statusbar->addPermanentWidget(m_coordinatesLabel);

    connect(ui->graphicsView, &CircuitGraphicsView::mouseScenePositionChanged,
            this, [this](QPointF pos) {
        m_coordinatesLabel->setText(
            tr("X: %1, Y: %2").arg(int(pos.x())).arg(int(pos.y())));
    });

    //-----------------------------------------
    // Simulation / DRC Log Panel
    //-----------------------------------------
    // پنجره اصلی هیچ منطقی درباره DRC یا شبیه‌سازی نمی‌داند؛ فقط پیام‌های
    // SimulationLogger را نمایش می‌دهد (بخش ۱۱.۳ مستند پروژه)

    connect(&SimulationLogger::instance(), &SimulationLogger::messageLogged,
            this, [this](LogLevel level, const QString &message)
    {
        QString color = "black";
        if (level == LogLevel::Warning) color = "#b8860b";
        else if (level == LogLevel::Error) color = "#c0392b";

        ui->logTextEdit->appendHtml(
            QString("<span style='color:%1'>%2</span>")
                .arg(color, message.toHtmlEscaped()));
    });

    connect(&SimulationLogger::instance(), &SimulationLogger::cleared,
            ui->logTextEdit, &QPlainTextEdit::clear);

    updateWindowTitle();
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
// Library Search Filter (بخش ۳.۲ مستند پروژه)
//////////////////////////////////////////////////////////

void MainWindow::on_lineEditSearch_textChanged(const QString &text)
{
    const QString needle = text.trimmed().toLower();

    for (int i = 0; i < ui->treeWidgetComponents->topLevelItemCount(); ++i) {
        QTreeWidgetItem *category = ui->treeWidgetComponents->topLevelItem(i);
        bool anyVisibleChild = false;

        for (int j = 0; j < category->childCount(); ++j) {
            QTreeWidgetItem *child = category->child(j);
            const bool matches = needle.isEmpty() || child->text(0).toLower().contains(needle);
            child->setHidden(!matches);
            if (matches)
                anyVisibleChild = true;
        }

        category->setHidden(!anyVisibleChild);
        if (!needle.isEmpty() && anyVisibleChild)
            category->setExpanded(true);
    }
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
            Wire *wire = dynamic_cast<Wire*>(item);

            if(undoStack)
            {
                if (wire)
                    undoStack->push(new DeleteWireCommand(scene, wire));
                else
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
        scene->cancelWireDrawing();

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

    //--------------------------------------
    // MIRROR → Undo/Redo Command
    //--------------------------------------
    if(event->key() == Qt::Key_M)
    {
        QList<QGraphicsItem*> items =
            scene->selectedItems();

        for(QGraphicsItem *item : items)
        {
            if (dynamic_cast<Wire*>(item))
                continue; // آینه‌کردن فقط برای قطعات معنا دارد، نه سیم‌ها

            if(undoStack)
            {
                undoStack->push(
                    new MirrorCommand(
                        scene,
                        item));
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
    // اگر این ابزار از قبل فعال بوده و کاربر دوباره روی دکمه کلیک کرده، یعنی می‌خواهد قطعه را تغییر دهد:
    if (toolManager->currentTool() == Tool::PlaceComponent) {
        openComponentSelectionDialog();
        return;
    }

    // در غیر این صورت، با یک‌بار کلیک، آخرین قطعه انتخاب شده سریعاً فعال می‌شود
    toolManager->setCurrentTool(Tool::PlaceComponent);
    toolManager->setComponentName(m_lastSelectedComponent);
    scene->beginComponentPlacement(m_lastSelectedComponent);

    qDebug() << "قطعه فعال شد:" << m_lastSelectedComponent;
}
void MainWindow::openComponentSelectionDialog()
{
    // ۱. ساختن یک لیست متنی خالی
    QStringList items;

    // ۲. گرفتن تمام قطعاتی که در LibraryManager ثبت کرده بودیم (مقاومت، خازن، دیود، منبع)
    const auto list = LibraryManager::components();
    for(const ComponentInfo &info : list) {
        items << info.name; // نام تک‌تک قطعات را داخل لیست می‌ریزیم
    }

    // ۳. این خط یک پنجره کوچک آماده روی صفحه باز می‌کند که لیست قطعات داخلش است
    bool ok;
    QString selectedItem = QInputDialog::getItem(this, "انتخاب قطعه",
                                                 "لطفاً قطعه مورد نظر خود را انتخاب کنید:",
                                                 items, 0, false, &ok);
    // ۴. اگر کاربر یک قطعه را انتخاب کرد و دکمه OK را زد:
    if (ok && !selectedItem.isEmpty()) {
        m_lastSelectedComponent = selectedItem; // نام قطعه جدید را ذخیره کن تا یادش بماند

        // ۵. فعال کردن قطعه انتخاب شده روی بوم مدار (دقیقاً بر اساس کدهای پروژه شما)
        toolManager->setCurrentTool(Tool::PlaceComponent);
        toolManager->setComponentName(selectedItem);
        scene->beginComponentPlacement(selectedItem);

        // ۶. تغییر نام پویا و زنده دکمه تولبار به نام قطعه انتخاب شده (جدید)
        ui->actionResistor->setText(selectedItem);
    }

}









void MainWindow::on_actionExportImage_triggered() {
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Export Image"), "", tr("PNG Image (*.png);;JPEG Image (*.jpg)"));

    if (!fileName.isEmpty()) {
        // محدوده مدار رسم شده را می‌گیرد (نه کل بوم خالی را)
        QRectF rect = scene->itemsBoundingRect();
        // اضافه کردن کمی حاشیه اطراف مدار
        rect.adjust(-20, -20, 20, 20);

        QImage image(rect.size().toSize(), QImage::Format_ARGB32);
        image.fill(Qt::white); // رنگ پس‌زمینه خروجی

        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing);
        scene->render(&painter, QRectF(), rect);
        painter.end();

        if (image.save(fileName)) {
            QMessageBox::information(this, tr("Success"), tr("Image exported successfully."));
        }
    }
}

//////////////////////////////////////////////////////////
// File Menu: New / Open / Save / Save As (بخش ۱۰.۱ مستند)
//////////////////////////////////////////////////////////

void MainWindow::updateWindowTitle()
{
    const QString projectName = m_currentFilePath.isEmpty()
        ? tr("Untitled")
        : QFileInfo(m_currentFilePath).fileName();

    setWindowTitle(QString("Proteus Clone - %1").arg(projectName));
}

void MainWindow::on_actionNew_triggered()
{
    if (!scene->components().isEmpty() || !scene->wires().isEmpty()) {
        const auto reply = QMessageBox::question(this, tr("پروژه جدید"),
            tr("مدار فعلی ذخیره نشده پاک خواهد شد. ادامه می‌دهید؟"),
            QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes)
            return;
    }

    scene->resetCircuit();
    undoStack->clear();
    m_currentFilePath.clear();
    updateWindowTitle();
    SimulationLogger::instance().clear();
}

void MainWindow::on_actionOpen_triggered()
{
    const QString path = QFileDialog::getOpenFileName(
        this, tr("باز کردن پروژه"), QString(),
        tr("Proteus Clone Project (*.pcproj *.json)"));

    if (path.isEmpty())
        return;

    openProjectFile(path);
}

void MainWindow::openProjectFile(const QString &path)
{
    QString error;
    if (ProjectSerializer::load(scene, path, &error)) {
        m_currentFilePath = path;
        undoStack->clear();
        updateWindowTitle();
        SimulationLogger::instance().clear();
        SimulationLogger::instance().log(LogLevel::Info,
            tr("پروژه با موفقیت بارگذاری شد: %1").arg(path));
    } else {
        QMessageBox::warning(this, tr("خطا در بارگذاری"), error);
    }
}

void MainWindow::setCanvasSize(const QSize &size)
{
    scene->setSceneRect(0, 0, size.width(), size.height());
}

void MainWindow::on_actionSave_triggered()
{
    if (m_currentFilePath.isEmpty()) {
        on_actionSaveAs_triggered();
        return;
    }

    QString error;
    if (ProjectSerializer::save(scene, m_currentFilePath, &error)) {
        SimulationLogger::instance().log(LogLevel::Info,
            tr("پروژه ذخیره شد: %1").arg(m_currentFilePath));
    } else {
        QMessageBox::warning(this, tr("خطا در ذخیره‌سازی"), error);
    }
}

void MainWindow::on_actionSaveAs_triggered()
{
    QString path = QFileDialog::getSaveFileName(
        this, tr("ذخیره پروژه با نام"), QString(),
        tr("Proteus Clone Project (*.pcproj)"));

    if (path.isEmpty())
        return;

    if (!path.endsWith(".pcproj", Qt::CaseInsensitive))
        path += ".pcproj";

    QString error;
    if (ProjectSerializer::save(scene, path, &error)) {
        m_currentFilePath = path;
        updateWindowTitle();
        SimulationLogger::instance().log(LogLevel::Info,
            tr("پروژه ذخیره شد: %1").arg(path));
    } else {
        QMessageBox::warning(this, tr("خطا در ذخیره‌سازی"), error);
    }
}

//////////////////////////////////////////////////////////
// Tools Menu: Design Rule Check (بخش ۱۱ مستند)
//////////////////////////////////////////////////////////

void MainWindow::on_actionCheckDesign_triggered()
{
    SimulationLogger::instance().clear();

    const QList<DrcIssue> issues = DesignRuleChecker::check(scene);
    for (const DrcIssue &issue : issues) {
        LogLevel level = LogLevel::Info;
        if (issue.severity == DrcSeverity::Warning) level = LogLevel::Warning;
        else if (issue.severity == DrcSeverity::Error) level = LogLevel::Error;

        SimulationLogger::instance().log(level, issue.message);
    }
}

//////////////////////////////////////////////////////////
// Simulation: Run / Pause / Stop / Step (بخش ۸ مستند)
//////////////////////////////////////////////////////////

void MainWindow::on_actionRun_triggered()
{
    simEngine->start();
}

void MainWindow::on_actionPause_triggered()
{
    simEngine->pause();
}

void MainWindow::on_actionStop_triggered()
{
    simEngine->stop();
}

void MainWindow::on_actionStep_triggered()
{
    simEngine->step();
}

void MainWindow::onSimulationStateChanged(SimulationState state)
{
    const bool running = (state == SimulationState::Running);
    const bool stopped = (state == SimulationState::Stopped);

    ui->actionRun->setEnabled(!running);
    ui->actionPause->setEnabled(running);
    ui->actionStop->setEnabled(!stopped);
    ui->actionStep->setEnabled(!running);

    // در حین اجرای پیوسته، تغییر مستقیم مدار (جابجایی/سیم‌کشی) گیج‌کننده و مستعد خطاست
    ui->treeWidgetComponents->setEnabled(!running);
}