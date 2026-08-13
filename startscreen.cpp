#include "startscreen.h"
#include "clickablecard.h"
#include "logofactory.h"
#include "iconfactory.h"
#include "recentprojectsmanager.h"
#include "newprojectwizard.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QToolButton>
#include <QButtonGroup>
#include <QStackedWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QAbstractItemView>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QSize>
#include <QSizePolicy>

StartScreen::StartScreen(QWidget *parent)
    : QDialog(parent)
{
    setModal(true);
    buildUi();
}

void StartScreen::buildUi()
{
    setWindowTitle(tr("ProteusClone"));
    setWindowIcon(LogoFactory::icon());
    resize(980, 620);
    setMinimumSize(860, 560);

    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(buildSidebar());

    m_stack = new QStackedWidget(this);
    m_stack->addWidget(buildStartPage());  // index 0
    m_stack->addWidget(buildAboutPage());  // index 1
    mainLayout->addWidget(m_stack, 1);

    QString qss = QStringLiteral(
        "QDialog { background-color: #F4F6F9; }"
        "#sidebar { background-color: #20242C; }"
        "#sidebarAppName { color: #F2F4F8; font-size: 16px; font-weight: 700; }"
        "#sidebarFootnote { color: #6E7686; font-size: 10px; }"
        "QToolButton#navButton {"
        "   color: #B7BECC; background: transparent; border: none;"
        "   text-align: left; padding: 10px 14px; font-size: 13px; border-radius: 6px;"
        "}"
        "QToolButton#navButton:hover { background-color: #2A2F3A; color: #F2F4F8; }"
        "QToolButton#navButton:checked { background-color: #286EC8; color: #FFFFFF; font-weight: 600; }"
        "#pageTitle { color: #20242C; font-size: 22px; font-weight: 700; }"
        "#pageSubtitle { color: #7A828F; font-size: 12px; }"
        "#sectionHeader { color: #33373F; font-size: 14px; font-weight: 600; }"
        "QListWidget#recentList { background: transparent; border: none; }"
        "QListWidget#recentList::item { border: none; }"
        "QListWidget#recentList::item:selected { background: transparent; }"
    );
    qss += ClickableCard::cardStyleSheet();
    setStyleSheet(qss);
}

QWidget *StartScreen::buildSidebar()
{
    auto *sidebar = new QFrame(this);
    sidebar->setObjectName(QStringLiteral("sidebar"));
    sidebar->setFixedWidth(220);

    auto *layout = new QVBoxLayout(sidebar);
    layout->setContentsMargins(18, 24, 18, 18);
    layout->setSpacing(4);

    auto *brandRow = new QHBoxLayout();
    brandRow->setSpacing(10);

    auto *logoLabel = new QLabel(sidebar);
    logoLabel->setPixmap(LogoFactory::pixmap(34));
    logoLabel->setFixedSize(34, 34);
    brandRow->addWidget(logoLabel);

    auto *nameLabel = new QLabel(QStringLiteral("ProteusClone"), sidebar);
    nameLabel->setObjectName(QStringLiteral("sidebarAppName"));
    brandRow->addWidget(nameLabel);
    brandRow->addStretch();

    layout->addLayout(brandRow);
    layout->addSpacing(26);

    auto *navGroup = new QButtonGroup(sidebar);
    navGroup->setExclusive(true);

    auto makeNavButton = [&](const QString &text, bool checked) {
        auto *btn = new QToolButton(sidebar);
        btn->setObjectName(QStringLiteral("navButton"));
        btn->setText(text);
        btn->setCheckable(true);
        btn->setChecked(checked);
        btn->setToolButtonStyle(Qt::ToolButtonTextOnly);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        navGroup->addButton(btn);
        layout->addWidget(btn);
        return btn;
    };

    QToolButton *startBtn = makeNavButton(tr("Start"), true);
    QToolButton *aboutBtn = makeNavButton(tr("About"), false);

    connect(startBtn, &QToolButton::clicked, this, &StartScreen::showStartPage);
    connect(aboutBtn, &QToolButton::clicked, this, &StartScreen::showAboutPage);

    layout->addStretch();

    auto *footnote = new QLabel(tr("Development Build"), sidebar);
    footnote->setObjectName(QStringLiteral("sidebarFootnote"));
    layout->addWidget(footnote);

    return sidebar;
}

QWidget *StartScreen::buildStartPage()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(36, 32, 36, 24);
    layout->setSpacing(18);

    auto *title = new QLabel(tr("Welcome to ProteusClone"), page);
    title->setObjectName(QStringLiteral("pageTitle"));
    layout->addWidget(title);

    auto *subtitle = new QLabel(
        tr("یک پروژه جدید بسازید، پروژه‌ای موجود را باز کنید یا از یک قالب آماده شروع کنید."), page);
    subtitle->setObjectName(QStringLiteral("pageSubtitle"));
    layout->addWidget(subtitle);

    auto *actionsRow = new QHBoxLayout();
    actionsRow->setSpacing(14);

    auto *newCard = new ClickableCard(
        tr("New Project"), tr("شروع یک پروژه خالی با نام و مسیر دلخواه"), page);
    newCard->setIconPixmap(IconFactory::newProjectIcon().pixmap(24, 24));
    connect(newCard, &ClickableCard::clicked, this, &StartScreen::onNewProjectClicked);

    auto *openCard = new ClickableCard(
        tr("Open Project"), tr("باز کردن یک فایل پروژه (pcproj.*) موجود از دیسک"), page);
    openCard->setIconPixmap(IconFactory::openProjectIcon().pixmap(24, 24));
    connect(openCard, &ClickableCard::clicked, this, &StartScreen::onOpenProjectClicked);

    auto *templateCard = new ClickableCard(
        tr("Template"), tr("ساخت پروژه جدید بر پایه یکی از قالب‌های آماده صفحه شماتیک"), page);
    templateCard->setIconPixmap(IconFactory::templatesIcon().pixmap(24, 24));
    connect(templateCard, &ClickableCard::clicked, this, &StartScreen::onTemplatesClicked);

    for (ClickableCard *card : {newCard, openCard, templateCard}) {
        card->setMinimumHeight(108);
        actionsRow->addWidget(card, 1);
    }
    layout->addLayout(actionsRow);

    auto *recentHeader = new QLabel(tr("پروژه‌های اخیر"), page);
    recentHeader->setObjectName(QStringLiteral("sectionHeader"));
    layout->addWidget(recentHeader);

    m_recentList = new QListWidget(page);
    m_recentList->setObjectName(QStringLiteral("recentList"));
    m_recentList->setFrameShape(QFrame::NoFrame);
    m_recentList->setSpacing(4);
    m_recentList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_recentList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_recentList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    connect(m_recentList, &QListWidget::itemActivated, this, &StartScreen::onRecentItemActivated);
    connect(m_recentList, &QListWidget::customContextMenuRequested,
            this, &StartScreen::onRecentContextMenuRequested);
    layout->addWidget(m_recentList, 1);

    m_emptyStateLabel = new QLabel(
        tr("هنوز پروژه‌ای وجود ندارد. یک پروژه جدید بسازید یا پروژه‌ای را باز کنید."), page);
    m_emptyStateLabel->setObjectName(QStringLiteral("pageSubtitle"));
    m_emptyStateLabel->setAlignment(Qt::AlignCenter);
    m_emptyStateLabel->setVisible(false);
    layout->addWidget(m_emptyStateLabel, 1);

    reloadRecentProjects();

    return page;
}

QWidget *StartScreen::buildAboutPage()
{
    auto *page = new QWidget(this);
    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(36, 60, 36, 36);
    layout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    layout->setSpacing(10);

    auto *logoLabel = new QLabel(page);
    logoLabel->setPixmap(LogoFactory::pixmap(84));
    logoLabel->setFixedSize(84, 84);
    logoLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(logoLabel, 0, Qt::AlignHCenter);

    auto *nameLabel = new QLabel(QStringLiteral("ProteusClone"), page);
    nameLabel->setObjectName(QStringLiteral("pageTitle"));
    nameLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(nameLabel);

    auto *tagline = new QLabel(
        tr("نرم‌افزار طراحی و شبیه‌سازی شماتیک مدارهای الکترونیکی"), page);
    tagline->setObjectName(QStringLiteral("pageSubtitle"));
    tagline->setAlignment(Qt::AlignCenter);
    layout->addWidget(tagline);

    layout->addSpacing(16);

    auto *builtWith = new QLabel(tr("Built with C++ & Qt6"), page);
    builtWith->setObjectName(QStringLiteral("sidebarFootnote"));
    builtWith->setStyleSheet(QStringLiteral("color:#8A909C; font-size:11px;"));
    builtWith->setAlignment(Qt::AlignCenter);
    layout->addWidget(builtWith);

    layout->addStretch();
    return page;
}

void StartScreen::showStartPage()
{
    m_stack->setCurrentIndex(0);
}

void StartScreen::showAboutPage()
{
    m_stack->setCurrentIndex(1);
}

void StartScreen::reloadRecentProjects()
{
    m_recentList->clear();

    const QStringList recents = RecentProjectsManager::recentProjects();

    for (const QString &path : recents) {
        const QFileInfo info(path);

        auto *item = new QListWidgetItem(m_recentList);
        item->setData(Qt::UserRole, path);
        item->setSizeHint(QSize(0, 56));

        auto *rowWidget = new QWidget(m_recentList);
        rowWidget->setObjectName(QStringLiteral("recentRow"));
        rowWidget->setStyleSheet(QStringLiteral(
            "#recentRow { background-color: #FFFFFF; border: 1px solid #E7EAF0; border-radius: 8px; }"
            "#recentRow:hover { border: 1px solid #286EC8; background-color: #F5F9FF; }"));

        auto *rowLayout = new QHBoxLayout(rowWidget);
        rowLayout->setContentsMargins(10, 6, 10, 6);
        rowLayout->setSpacing(10);

        auto *iconLabel = new QLabel(rowWidget);
        iconLabel->setPixmap(IconFactory::newProjectIcon().pixmap(20, 20));
        rowLayout->addWidget(iconLabel);

        auto *textLayout = new QVBoxLayout();
        textLayout->setSpacing(2);

        auto *nameLabel = new QLabel(info.completeBaseName(), rowWidget);
        nameLabel->setStyleSheet(QStringLiteral("font-weight:600; font-size:12px; color:#22262E;"));
        auto *pathLabel = new QLabel(QDir::toNativeSeparators(info.absolutePath()), rowWidget);
        pathLabel->setStyleSheet(QStringLiteral("font-size:10px; color:#8A909C;"));

        textLayout->addWidget(nameLabel);
        textLayout->addWidget(pathLabel);
        rowLayout->addLayout(textLayout, 1);

        auto *dateLabel = new QLabel(
            info.lastModified().toString(QStringLiteral("yyyy-MM-dd hh:mm")), rowWidget);
        dateLabel->setStyleSheet(QStringLiteral("font-size:10px; color:#8A909C;"));
        rowLayout->addWidget(dateLabel);

        m_recentList->setItemWidget(item, rowWidget);
    }

    const bool empty = recents.isEmpty();
    m_recentList->setVisible(!empty);
    m_emptyStateLabel->setVisible(empty);
}

void StartScreen::onNewProjectClicked()
{
    runNewProjectWizard(false);
}

void StartScreen::onTemplatesClicked()
{
    runNewProjectWizard(true);
}

void StartScreen::runNewProjectWizard(bool preferSchematic)
{
    NewProjectWizard wizard(preferSchematic, this);
    if (wizard.exec() == QDialog::Accepted)
        finishWith(wizard.createdProjectFilePath());
}

void StartScreen::onOpenProjectClicked()
{
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Open Project"), QString(),
        tr("Proteus Clone Project (*.pcproj *.json)"));

    if (path.isEmpty())
        return;

    finishWith(path);
}

void StartScreen::onRecentItemActivated(QListWidgetItem *item)
{
    if (!item)
        return;

    const QString path = item->data(Qt::UserRole).toString();

    if (!QFileInfo::exists(path)) {
        QMessageBox::warning(this, tr("فایل یافت نشد"),
            tr("فایل پروژه \"%1\" دیگر روی دیسک وجود ندارد و از فهرست حذف می‌شود.")
                .arg(QDir::toNativeSeparators(path)));
        RecentProjectsManager::removeRecentProject(path);
        reloadRecentProjects();
        return;
    }

    finishWith(path);
}

void StartScreen::onRecentContextMenuRequested(const QPoint &pos)
{
    QListWidgetItem *item = m_recentList->itemAt(pos);
    if (!item)
        return;

    QMenu menu(this);
    QAction *removeAction = menu.addAction(tr("حذف از فهرست"));
    QAction *chosen = menu.exec(m_recentList->viewport()->mapToGlobal(pos));
    if (chosen == removeAction) {
        RecentProjectsManager::removeRecentProject(item->data(Qt::UserRole).toString());
        reloadRecentProjects();
    }
}

void StartScreen::finishWith(const QString &filePath)
{
    m_chosenFilePath = filePath;
    accept();
}
