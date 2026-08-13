#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "librarymanager.h"
#include "circuitscene.h"
#include "circuitgraphicsview.h"
#include "componentfactory.h"
#include "iconfactory.h"
#include "pickdevicesdialog.h"
#include "io/projectserializer.h"
#include "recentprojectsmanager.h"
#include "logofactory.h"
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
#include <QToolButton>
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

    setWindowIcon(LogoFactory::icon());

    //-----------------------------------------
    // Component Library
    //-----------------------------------------

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

    // مقدار پیش‌فرض کادر شماتیک/آبی (بخش ۱ درخواست کاربر) - اگر main.cpp بلافاصله
    // setCanvasSize واقعی را با انتخاب کاربر صدا نزند (مثلاً هنگام Open Existing
    // یک پروژه قدیمی که اندازه‌ی کادر را ذخیره نکرده)، همین مقدار باقی می‌ماند.
    setCanvasSize(QSize(3000, 3000));

    simEngine = new SimulationEngine(scene, this);
    connect(simEngine, &SimulationEngine::stateChanged,
            this, &MainWindow::onSimulationStateChanged);
    onSimulationStateChanged(SimulationState::Stopped);

    ui->actionResistor->setText("Components");
    ui->actionResistor->setToolTip(tr("Place Component (last used: %1)").arg(m_lastSelectedComponent));

    //-----------------------------------------
    // آیکون‌ها (بخش «تکمیل ظاهری» - شبیه‌سازی نوار ابزارهای واقعی پروتئوس،
    // به IconFactory واگذار شده تا هیچ فایل تصویری خارجی لازم نباشد)
    //-----------------------------------------

    ui->actionNew->setIcon(IconFactory::newProjectIcon());
    ui->actionOpen->setIcon(IconFactory::openProjectIcon());
    ui->actionSave->setIcon(IconFactory::saveProjectIcon());
    ui->actionSaveAs->setIcon(IconFactory::saveAsIcon());
    ui->actionExportImage->setIcon(IconFactory::exportImageIcon());

    ui->actionSelect->setIcon(IconFactory::selectIcon());
    ui->actionWire->setIcon(IconFactory::wireIcon());
    ui->actionResistor->setIcon(IconFactory::placeComponentIcon());
    ui->actionRotateCW->setIcon(IconFactory::rotateCwIcon());
    ui->actionRotateCCW->setIcon(IconFactory::rotateCcwIcon());
    ui->actionMirror->setIcon(IconFactory::mirrorIcon());

    ui->actionCheckDesign->setIcon(IconFactory::checkDesignIcon());
    ui->actionPickParts->setIcon(IconFactory::pickPartsIcon());
    ui->pushButtonPickParts->setIcon(IconFactory::pickPartsIcon());
    ui->actionRun->setIcon(IconFactory::runIcon());
    ui->actionPause->setIcon(IconFactory::pauseIcon());
    ui->actionStop->setIcon(IconFactory::stopIcon());
    ui->actionStep->setIcon(IconFactory::stepIcon());

    ui->actionZoomIn->setIcon(IconFactory::zoomInIcon());
    ui->actionZoomOut->setIcon(IconFactory::zoomOutIcon());
    ui->actionZoomReset->setIcon(IconFactory::zoomResetIcon());

    // Undo/Redo با متن پویا (شرح آخرین عمل) از خودِ QUndoStack ساخته می‌شوند،
    // پس نمی‌توانند از قبل توی مستند Designer (.ui) تعریف شده باشند؛ اینجا هم
    // آیکون می‌گیرند و هم کنار Save در نوار بالا قرار می‌گیرند - دقیقاً مطابق
    // چیدمان نوار ابزار اصلی پروتئوس واقعی (New/Open/Save | Undo/Redo | Zoom).
    QAction *undoAction = undoStack->createUndoAction(this, tr("Undo"));
    QAction *redoAction = undoStack->createRedoAction(this, tr("Redo"));
    undoAction->setIcon(IconFactory::undoIcon());
    redoAction->setIcon(IconFactory::redoIcon());
    ui->mainToolBar->insertAction(ui->actionZoomIn, undoAction);
    ui->mainToolBar->insertAction(ui->actionZoomIn, redoAction);
    ui->mainToolBar->insertSeparator(ui->actionZoomIn);

    //-----------------------------------------
    // دکمه‌های Run/Pause/Stop/Step روی نوار وضعیت پایین صفحه (نه نوار بالا) -
    // دقیقاً همان جایی که نرم‌افزار واقعی پروتئوس این دکمه‌ها را نشان می‌دهد.
    //-----------------------------------------
    auto addStatusBarToolButton = [this](QAction *action) {
        auto *button = new QToolButton(this);
        button->setDefaultAction(action);
        button->setAutoRaise(true);
        ui->statusbar->addWidget(button);
        return button;
    };
    addStatusBarToolButton(ui->actionRun);
    addStatusBarToolButton(ui->actionPause);
    addStatusBarToolButton(ui->actionStop);
    addStatusBarToolButton(ui->actionStep);

    ui->graphicsView->setScene(scene);

    // یک قطعه پیش‌فرض (Resistor) از ابتدا در لیست قطعات فعال باشد تا کاربر
    // مجبور نباشد برای اولین قطعه هم حتماً پنجره Pick Devices را باز کند.
    addComponentToActiveList(m_lastSelectedComponent);

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
// لیست قطعات فعال (بخش ۳.۴ مستند پروژه) - پنل کناری برای دسترسی سریع به
// قطعات پرکاربرد؛ کتابخانه‌ی کامل از طریق PickDevicesDialog (بخش ۳.۱-۳.۳)
// در دسترس است، نه یک درخت همیشه‌باز.
//////////////////////////////////////////////////////////

void MainWindow::addComponentToActiveList(const QString &componentName)
{
    // اگر از قبل توی لیست بود، دوباره اضافه نکن (بدون آیتم تکراری)
    const QList<QListWidgetItem *> existing =
        ui->listWidgetActiveParts->findItems(componentName, Qt::MatchExactly);
    if (!existing.isEmpty())
        return;

    ui->listWidgetActiveParts->addItem(componentName);
}

void MainWindow::armComponentForPlacement(const QString &componentName)
{
    toolManager->setCurrentTool(Tool::PlaceComponent);
    toolManager->setComponentName(componentName);
    scene->beginComponentPlacement(componentName);

    m_lastSelectedComponent = componentName;
    ui->actionResistor->setText(componentName);
    ui->actionResistor->setToolTip(tr("Place Component (last used: %1)").arg(componentName));
}

void MainWindow::on_listWidgetActiveParts_itemClicked(QListWidgetItem *item)
{
    if (!item)
        return;
    // طبق بخش ۳.۴ مستند: «با کلیک روی هر آیتم در این لیست، آن قطعه آماده‌ی
    // قرارگیری روی بوم می‌شود»
    armComponentForPlacement(item->text());
}

void MainWindow::on_pushButtonRemovePart_clicked()
{
    // بخش ۳.۴ مستند: «طبیعتاً امکان حذف قطعات از این لیست هم باید وجود داشته باشد»
    QListWidgetItem *item = ui->listWidgetActiveParts->currentItem();
    if (!item)
        return;
    delete ui->listWidgetActiveParts->takeItem(ui->listWidgetActiveParts->row(item));
}

void MainWindow::on_actionPickParts_triggered()
{
    openComponentSelectionDialog();
}

void MainWindow::on_pushButtonPickParts_clicked()
{
    openComponentSelectionDialog();
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
    armComponentForPlacement(m_lastSelectedComponent);
}
//////////////////////////////////////////////////////////
// نوار ابزار حالت‌ها: چرخش و قرینه‌سازی (بخش ۴.۴/۴.۵ مستند) - همان منطق
// keyPressEvent (کلیدهای R/M)، فقط این‌بار از روی دکمه‌های آیکونی نوار کناری
//////////////////////////////////////////////////////////

void MainWindow::on_actionRotateCW_triggered()
{
    for (QGraphicsItem *item : scene->selectedItems())
        if (undoStack)
            undoStack->push(new RotateCommand(scene, item, 90));
}

void MainWindow::on_actionRotateCCW_triggered()
{
    for (QGraphicsItem *item : scene->selectedItems())
        if (undoStack)
            undoStack->push(new RotateCommand(scene, item, -90));
}

void MainWindow::on_actionMirror_triggered()
{
    for (QGraphicsItem *item : scene->selectedItems()) {
        if (dynamic_cast<Wire*>(item))
            continue; // آینه‌کردن فقط برای قطعات معنا دارد، نه سیم‌ها
        if (undoStack)
            undoStack->push(new MirrorCommand(scene, item));
    }
}

void MainWindow::openComponentSelectionDialog()
{
    // پنجره Pick Devices واقعی (بخش ۳.۱-۳.۳ مستند) - جایگزین QInputDialog::getItem
    // ساده‌ی قبلی؛ شامل جستجوی زنده (روی نام و دسته)، دسته‌بندی درختی و پیش‌نمایش زنده.
    PickDevicesDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    const QString selectedItem = dialog.selectedComponentName();
    if (selectedItem.isEmpty())
        return;

    armComponentForPlacement(selectedItem);
    addComponentToActiveList(selectedItem); // بخش ۳.۴ مستند: به لیست قطعات فعال هم اضافه شود
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
        // ثبت در فهرست «پروژه‌های اخیر» صفحه Start - این تنها نقطه‌ای است که
        // هر بار یک پروژه با موفقیت باز می‌شود (چه از StartScreen موقع اجرای
        // برنامه، چه بعداً از منوی File > Open) از آن عبور می‌کند.
        RecentProjectsManager::addRecentProject(path);
    } else {
        QMessageBox::warning(this, tr("خطا در بارگذاری"), error);
    }
}

void MainWindow::setCanvasSize(const QSize &size)
{
    // کادر آبی (محدوده قابل‌شبیه‌سازی) دقیقاً هم‌اندازه با مقدار انتخاب‌شده کاربر
    // در StartupDialog می‌شود؛ خودِ CircuitScene::setSchematicRect هم sceneRect
    // واقعی صحنه را برای نمایش پس‌زمینه شطرنجی اطراف کادر بزرگ‌تر تنظیم می‌کند
    // (بخش ۱ درخواست کاربر).
    scene->setSchematicRect(QRectF(0, 0, size.width(), size.height()));
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
        RecentProjectsManager::addRecentProject(path); // ثبت در فهرست پروژه‌های اخیر صفحه Start
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

    // در حین اجرای پیوسته، تغییر مستقیم مدار (جابجایی/سیم‌کشی/قطعه جدید) گیج‌کننده و مستعد خطاست
    ui->listWidgetActiveParts->setEnabled(!running);
    ui->pushButtonPickParts->setEnabled(!running);
    ui->pushButtonRemovePart->setEnabled(!running);
    ui->actionPickParts->setEnabled(!running);
}