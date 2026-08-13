#include "newprojectwizard.h"
#include "clickablecard.h"
#include "logofactory.h"
#include "iconfactory.h"
#include "circuitscene.h"
#include "io/projectserializer.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QPainter>
#include <QPen>
#include <QAbstractButton>
#include <utility>

namespace {

// --- قالب‌های آماده صفحه شماتیک (بخش «صفحه شروع زیبا» - New Project Wizard) ---
// اندازه‌ها بر پایه ابعاد استاندارد کاغذ (میلی‌متر)، با مقیاس ۱۰ واحد صحنه به
// ازای هر میلی‌متر (هم‌خانواده با پیش‌فرض قبلی برنامه: ۳۰۰۰×۳۰۰۰ برای یک بومِ
// عمومی) به پیکسل صحنه تبدیل شده‌اند.
struct PaperTemplate {
    const char *label;
    const char *dims;
    bool landscape;
    QSize pixelSize;
};

const PaperTemplate kTemplates[] = {
    { "Landscape A4",     "297 x 210 mm", true,  QSize(2970, 2100) },
    { "Landscape A5",     "210 x 148 mm", true,  QSize(2100, 1480) },
    { "Landscape Letter", "279 x 216 mm", true,  QSize(2790, 2160) },
    { "Landscape A3",     "420 x 297 mm", true,  QSize(4200, 2970) },
    { "Portrait A3",      "297 x 420 mm", false, QSize(2970, 4200) },
    { "Portrait A4",      "210 x 297 mm", false, QSize(2100, 2970) },
    { "Portrait A2",      "420 x 594 mm", false, QSize(4200, 5940) },
};
const int kTemplateCount = int(sizeof(kTemplates) / sizeof(kTemplates[0]));

// آیکون کوچک هر کارتِ قالب: یک مستطیل کاغذ با جهت درست (افقی/عمودی) + چند خط
// کوتاه یادآور خطوط شماتیک.
QPixmap paperIcon(bool landscape)
{
    QPixmap pm(28, 28);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor(55, 55, 60), 1.4));
    p.setBrush(Qt::white);

    const QRectF rect = landscape ? QRectF(2, 6, 24, 16) : QRectF(6, 2, 16, 24);
    p.drawRoundedRect(rect, 1.5, 1.5);

    p.setPen(QPen(QColor(40, 110, 200), 1.1));
    if (landscape) {
        p.drawLine(QPointF(rect.left() + 4, rect.center().y() - 3), QPointF(rect.left() + 14, rect.center().y() - 3));
        p.drawLine(QPointF(rect.left() + 4, rect.center().y() + 3), QPointF(rect.left() + 10, rect.center().y() + 3));
    } else {
        p.drawLine(QPointF(rect.center().x() - 3, rect.top() + 5), QPointF(rect.center().x() - 3, rect.top() + 15));
        p.drawLine(QPointF(rect.center().x() + 3, rect.top() + 5), QPointF(rect.center().x() + 3, rect.top() + 11));
    }
    return pm;
}

} // namespace

//////////////////////////////////////////////////////////
// ProjectInfoPage
//////////////////////////////////////////////////////////

ProjectInfoPage::ProjectInfoPage(QWidget *parent)
    : QWizardPage(parent)
{
    // عنوان/زیرعنوان استاندارد QWizardPage (بنر بالای صفحه) عمداً خالی گذاشته
    // شده و به‌جایش عنوان خودمان داخل چیدمان صفحه چیده می‌شود؛ چون رسم بنر در
    // سبک‌های مختلف QWizard (Aero/Mac/Vista) بین پلتفرم‌ها خیلی متفاوت به نظر
    // می‌رسد و کنترل کامل روی ظاهرش سخت است. به همین دلیل NewProjectWizard هم
    // صراحتاً QWizard::ClassicStyle را انتخاب کرده (نگاه کن newprojectwizard.cpp).
    setTitle(QString());
    setSubTitle(QString());

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(30, 24, 30, 10);
    layout->setSpacing(12);

    auto *heading = new QLabel(tr("New Project"), this);
    heading->setStyleSheet(QStringLiteral("font-size:19px; font-weight:700; color:#20242C;"));
    layout->addWidget(heading);

    auto *desc = new QLabel(tr("نام و مسیر ذخیره‌سازی پروژه جدید را مشخص کنید."), this);
    desc->setStyleSheet(QStringLiteral("color:#7A828F; font-size:12px;"));
    layout->addWidget(desc);

    layout->addSpacing(10);

    auto *nameLabel = new QLabel(tr("Project Name"), this);
    nameLabel->setStyleSheet(QStringLiteral("font-weight:600; font-size:12px; color:#33373F;"));
    layout->addWidget(nameLabel);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText(tr("مثلاً MyCircuit"));
    layout->addWidget(m_nameEdit);

    // با ثبت این فیلد به‌عنوان mandatory (پسوند *)، خودِ QWizard دکمه Next را
    // تا وقتی خالی است غیرفعال نگه می‌دارد - نیازی به isComplete() دستی نیست.
    registerField(QStringLiteral("projectName*"), m_nameEdit);

    layout->addSpacing(6);

    auto *dirLabel = new QLabel(tr("Location"), this);
    dirLabel->setStyleSheet(QStringLiteral("font-weight:600; font-size:12px; color:#33373F;"));
    layout->addWidget(dirLabel);

    auto *dirRow = new QHBoxLayout();
    m_dirEdit = new QLineEdit(this);

    const QString docsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString defaultDir = docsPath.isEmpty()
        ? QDir::homePath()
        : docsPath + QStringLiteral("/ProteusClone Projects");
    m_dirEdit->setText(QDir::toNativeSeparators(defaultDir));

    auto *browseButton = new QPushButton(tr("Browse..."), this);
    dirRow->addWidget(m_dirEdit, 1);
    dirRow->addWidget(browseButton);
    layout->addLayout(dirRow);

    registerField(QStringLiteral("projectDir*"), m_dirEdit);

    m_previewLabel = new QLabel(this);
    m_previewLabel->setWordWrap(true);
    m_previewLabel->setStyleSheet(QStringLiteral("color:#8A909C; font-size:11px;"));
    layout->addWidget(m_previewLabel);

    layout->addStretch();

    connect(m_nameEdit, &QLineEdit::textChanged, this, &ProjectInfoPage::updatePreview);
    connect(m_dirEdit, &QLineEdit::textChanged, this, &ProjectInfoPage::updatePreview);
    connect(browseButton, &QPushButton::clicked, this, [this] {
        const QString chosen = QFileDialog::getExistingDirectory(
            this, tr("Choose Project Location"), m_dirEdit->text());
        if (!chosen.isEmpty())
            m_dirEdit->setText(QDir::toNativeSeparators(chosen));
    });

    updatePreview();
}

QString ProjectInfoPage::projectName() const
{
    return m_nameEdit->text().trimmed();
}

QString ProjectInfoPage::projectDirectory() const
{
    return m_dirEdit->text().trimmed();
}

void ProjectInfoPage::updatePreview()
{
    const QString name = projectName();
    const QString dir = projectDirectory();

    if (name.isEmpty() || dir.isEmpty()) {
        m_previewLabel->setText(tr("نام و مسیر پروژه را وارد کنید."));
        return;
    }

    QString fileName = name;
    if (!fileName.endsWith(QStringLiteral(".pcproj"), Qt::CaseInsensitive))
        fileName += QStringLiteral(".pcproj");

    const QString fullPath = QDir(dir).filePath(fileName);
    m_previewLabel->setText(
        tr("فایل پروژه در این مسیر ساخته می‌شود:\n%1").arg(QDir::toNativeSeparators(fullPath)));
}

bool ProjectInfoPage::validatePage()
{
    const QString name = projectName();
    const QString dir = projectDirectory();

    if (name.isEmpty()) {
        QMessageBox::warning(this, tr("نام پروژه خالی است"), tr("لطفاً یک نام برای پروژه وارد کنید."));
        return false;
    }

    static const QString invalidChars = QStringLiteral("\\/:*?\"<>|");
    for (const QChar ch : invalidChars) {
        if (name.contains(ch)) {
            QMessageBox::warning(this, tr("نام پروژه نامعتبر است"),
                tr("نام پروژه نباید شامل هیچ‌کدام از این کاراکترها باشد: %1").arg(invalidChars));
            return false;
        }
    }

    if (dir.isEmpty()) {
        QMessageBox::warning(this, tr("مسیر انتخاب نشده"), tr("لطفاً مسیر ذخیره‌سازی پروژه را انتخاب کنید."));
        return false;
    }

    QDir targetDir(dir);
    if (!targetDir.exists()) {
        const auto reply = QMessageBox::question(this, tr("ساخت پوشه"),
            tr("پوشه \"%1\" وجود ندارد. ساخته شود؟").arg(QDir::toNativeSeparators(dir)),
            QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes)
            return false;
        if (!QDir().mkpath(dir)) {
            QMessageBox::warning(this, tr("خطا"), tr("امکان ساخت پوشه \"%1\" وجود ندارد.").arg(dir));
            return false;
        }
    }

    QString fileName = name;
    if (!fileName.endsWith(QStringLiteral(".pcproj"), Qt::CaseInsensitive))
        fileName += QStringLiteral(".pcproj");
    const QString fullPath = QDir(dir).filePath(fileName);

    if (QFileInfo::exists(fullPath)) {
        const auto reply = QMessageBox::question(this, tr("فایل موجود است"),
            tr("فایلی با نام \"%1\" از قبل در این مسیر وجود دارد. جایگزین شود؟").arg(fileName),
            QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes)
            return false;
    }

    return true;
}

//////////////////////////////////////////////////////////
// SchematicSizePage
//////////////////////////////////////////////////////////

SchematicSizePage::SchematicSizePage(bool preselectSchematic, QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(QString());
    setSubTitle(QString());

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(30, 24, 30, 10);
    layout->setSpacing(12);

    auto *heading = new QLabel(tr("Schematic Size"), this);
    heading->setStyleSheet(QStringLiteral("font-size:19px; font-weight:700; color:#20242C;"));
    layout->addWidget(heading);

    auto *desc = new QLabel(
        tr("می‌خواهید این پروژه از همین حالا یک صفحه شماتیک با ابعاد استاندارد داشته باشد؟"), this);
    desc->setStyleSheet(QStringLiteral("color:#7A828F; font-size:12px;"));
    desc->setWordWrap(true);
    layout->addWidget(desc);

    auto *modeRow = new QHBoxLayout();
    modeRow->setSpacing(14);

    m_noSchematicCard = new ClickableCard(
        tr("No Schematic"), tr("پروژه بدون صفحه شماتیک از پیش تعیین‌شده ساخته شود"), this);
    m_noSchematicCard->setIconPixmap(IconFactory::newProjectIcon().pixmap(24, 24));
    m_noSchematicCard->setCheckable(true);
    m_noSchematicCard->setMinimumHeight(90);

    m_createSchematicCard = new ClickableCard(
        tr("Create Schematic"), tr("یک صفحه شماتیک با ابعاد استاندارد کاغذ انتخاب کنید"), this);
    m_createSchematicCard->setIconPixmap(IconFactory::templatesIcon().pixmap(24, 24));
    m_createSchematicCard->setCheckable(true);
    m_createSchematicCard->setMinimumHeight(90);

    modeRow->addWidget(m_noSchematicCard, 1);
    modeRow->addWidget(m_createSchematicCard, 1);
    layout->addLayout(modeRow);

    connect(m_noSchematicCard, &ClickableCard::clicked, this, [this] { selectMode(false); });
    connect(m_createSchematicCard, &ClickableCard::clicked, this, [this] { selectMode(true); });

    auto *templateLabel = new QLabel(tr("Templates"), this);
    templateLabel->setStyleSheet(QStringLiteral("font-weight:600; font-size:12px; color:#33373F;"));
    layout->addWidget(templateLabel);

    m_templateGrid = new QWidget(this);
    auto *gridLayout = new QGridLayout(m_templateGrid);
    gridLayout->setSpacing(10);
    gridLayout->setContentsMargins(0, 0, 0, 0);

    const int columns = 4;
    for (int i = 0; i < kTemplateCount; ++i) {
        const PaperTemplate &t = kTemplates[i];
        auto *card = new ClickableCard(
            QString::fromLatin1(t.label), QString::fromLatin1(t.dims), m_templateGrid);
        card->setIconPixmap(paperIcon(t.landscape));
        card->setCheckable(true);
        card->setMinimumHeight(96);
        m_templateCards.append(card);

        const QSize size = t.pixelSize;
        const QString name = QString::fromLatin1(t.label);
        connect(card, &ClickableCard::clicked, this, [this, card, name, size] {
            selectTemplate(card, name, size);
        });

        gridLayout->addWidget(card, i / columns, i % columns);
    }
    layout->addWidget(m_templateGrid);
    layout->addStretch();

    selectMode(preselectSchematic);
    if (preselectSchematic && !m_templateCards.isEmpty()) {
        selectTemplate(m_templateCards.first(),
                        QString::fromLatin1(kTemplates[0].label), kTemplates[0].pixelSize);
    }
}

void SchematicSizePage::selectMode(bool createSchematic)
{
    m_createSchematic = createSchematic;
    m_noSchematicCard->setChecked(!createSchematic);
    m_createSchematicCard->setChecked(createSchematic);

    m_templateGrid->setEnabled(createSchematic);
    for (ClickableCard *card : std::as_const(m_templateCards))
        card->setEnabled(createSchematic);

    emit completeChanged();
}

void SchematicSizePage::selectTemplate(ClickableCard *card, const QString &name, QSize size)
{
    for (ClickableCard *c : std::as_const(m_templateCards))
        c->setChecked(c == card);

    m_selectedTemplateName = name;
    m_selectedCanvasSize = size;
    emit completeChanged();
}

bool SchematicSizePage::isComplete() const
{
    if (!m_createSchematic)
        return true;
    return !m_selectedTemplateName.isEmpty();
}

//////////////////////////////////////////////////////////
// NewProjectWizard
//////////////////////////////////////////////////////////

NewProjectWizard::NewProjectWizard(bool preferSchematic, QWidget *parent)
    : QWizard(parent)
{
    setWindowTitle(tr("New Project - ProteusClone"));
    setWindowIcon(LogoFactory::icon());

    // ClassicStyle عمداً انتخاب شده (نه ModernStyle/AeroStyle) چون بنر رنگی
    // بالای صفحه در آن سبک‌ها روی پلتفرم‌های مختلف خیلی متفاوت رسم می‌شود و
    // کنترل کامل روی ظاهرش با QSS ساده نیست؛ ClassicStyle یک صفحه ساده و
    // یک‌دست می‌دهد که خودمان کامل استایلش می‌دهیم.
    setWizardStyle(QWizard::ClassicStyle);
    setOption(QWizard::HaveHelpButton, true);
    setOption(QWizard::HelpButtonOnRight, false);

    setButtonText(QWizard::NextButton, tr("Next"));
    setButtonText(QWizard::BackButton, tr("Back"));
    setButtonText(QWizard::CancelButton, tr("Cancel"));
    setButtonText(QWizard::HelpButton, tr("Help"));
    setButtonText(QWizard::FinishButton, tr("Finish"));

    m_infoPage = new ProjectInfoPage(this);
    m_sizePage = new SchematicSizePage(preferSchematic, this);

    addPage(m_infoPage);
    addPage(m_sizePage);

    // برای رنگ‌آمیزی دکمه «کنش اصلی» (Next روی صفحه‌های میانی، Finish روی صفحه
    // آخر) به‌جای تطبیق QSS بر اساس متن نمایشی دکمه (که با تغییر زبان/متن می‌شکند)،
    // یک objectName مشترک روی هر دو ویجت دکمه گذاشته می‌شود - QWizard این دو را
    // به‌صورت دو دکمه جدا نگه می‌دارد و بسته به صفحه فعلی فقط یکی را نمایش می‌دهد.
    // این کار بعد از addPage() انجام می‌شود تا مطمئن باشیم دکمه‌های استاندارد
    // ویزارد کاملاً ساخته شده‌اند.
    if (QAbstractButton *nextBtn = button(QWizard::NextButton))
        nextBtn->setObjectName(QStringLiteral("wizardPrimaryButton"));
    if (QAbstractButton *finishBtn = button(QWizard::FinishButton))
        finishBtn->setObjectName(QStringLiteral("wizardPrimaryButton"));

    resize(720, 640);

    QString qss = QStringLiteral(
        "QWizard { background-color: #FFFFFF; }"
        "QLineEdit {"
        "   padding: 6px 8px; border: 1px solid #D5D9E0; border-radius: 6px; font-size: 12px;"
        "}"
        "QLineEdit:focus { border: 1px solid #286EC8; }"
        "QPushButton {"
        "   padding: 6px 14px; border-radius: 6px; font-size: 12px;"
        "   border: 1px solid #D5D9E0; background-color: #FFFFFF;"
        "}"
        "QPushButton:hover { border: 1px solid #286EC8; }"
        "#wizardPrimaryButton {"
        "   background-color: #286EC8; border: 1px solid #286EC8; color: #FFFFFF; font-weight: 600;"
        "}"
        "#wizardPrimaryButton:hover { background-color: #1F5AA8; }"
    );
    qss += ClickableCard::cardStyleSheet();
    setStyleSheet(qss);

    connect(this, &QWizard::helpRequested, this, &NewProjectWizard::onHelpRequested);
}

void NewProjectWizard::onHelpRequested()
{
    QString text;
    switch (currentId()) {
    case 0:
        text = tr("نام پروژه و مسیری که فایل .pcproj در آن ذخیره می‌شود را مشخص کنید. "
                   "با دکمه Browse می‌توانید یک پوشه از سیستم انتخاب کنید.");
        break;
    case 1:
        text = tr("اگر نیازی به صفحه شماتیک با اندازه مشخص ندارید، گزینه No Schematic را انتخاب کنید. "
                   "در غیر این صورت روی Create Schematic بزنید و یکی از قالب‌های استاندارد کاغذ را از پایین صفحه انتخاب کنید.");
        break;
    default:
        text = tr("برای ادامه مراحل از دکمه‌های Next/Back استفاده کنید.");
    }
    QMessageBox::information(this, tr("Help"), text);
}

void NewProjectWizard::accept()
{
    const QString name = m_infoPage->projectName();
    const QString dir = m_infoPage->projectDirectory();

    QString fileName = name;
    if (!fileName.endsWith(QStringLiteral(".pcproj"), Qt::CaseInsensitive))
        fileName += QStringLiteral(".pcproj");
    const QString fullPath = QDir(dir).filePath(fileName);

    // یک CircuitScene موقت و خالی فقط برای این‌که ProjectSerializer::save همان
    // مسیر واقعی Save/Load برنامه را طی کند (تا منطق نوشتن JSON پروژه در دو جای
    // مختلف تکرار نشود). این شیء هرگز به هیچ View ای وصل نمی‌شود و بلافاصله
    // بعد از ساخت فایل، در پایان همین تابع آزاد می‌شود.
    CircuitScene tempScene;
    if (m_sizePage->schematicEnabled()) {
        const QSize size = m_sizePage->selectedCanvasSize();
        // یکپارچه‌سازی: ProjectSerializer::save اندازه‌ی کادر را از
        // schematicRect() می‌خواند نه از sceneRect خام (نگاه کنید به
        // circuitscene.h/cpp و io/projectserializer.cpp) - در نتیجه اینجا هم
        // باید از همان مسیر عبور کند، وگرنه اندازه‌ی انتخاب‌شده در ویزارد هرگز
        // در فایل .pcproj تازه‌ساخته‌شده ذخیره نمی‌شد.
        tempScene.setSchematicRect(QRectF(0, 0, size.width(), size.height()));
    }

    QString error;
    if (!ProjectSerializer::save(&tempScene, fullPath, &error)) {
        QMessageBox::warning(this, tr("خطا در ساخت پروژه"),
            tr("امکان ساخت فایل پروژه وجود نداشت:\n%1").arg(error));
        return; // ویزارد باز می‌ماند تا کاربر مسیر/نام دیگری انتخاب کند
    }

    m_createdFilePath = fullPath;
    QWizard::accept();
}
