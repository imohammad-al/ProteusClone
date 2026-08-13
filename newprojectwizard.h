#ifndef NEWPROJECTWIZARD_H
#define NEWPROJECTWIZARD_H

#include <QWizard>
#include <QWizardPage>
#include <QSize>
#include <QString>
#include <QList>

class QLineEdit;
class QLabel;
class ClickableCard;

// صفحه ۱ ویزارد: نام پروژه + مسیر ذخیره‌سازی روی دیسک.
class ProjectInfoPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ProjectInfoPage(QWidget *parent = nullptr);

    QString projectName() const;
    QString projectDirectory() const;

    // اعتبارسنجی عمیق (کاراکترهای غیرمجاز، وجود/ساخت پوشه، تایید جایگزینی فایل
    // موجود) فقط موقع زدن Next اجرا می‌شود؛ isComplete() (فعال/غیرفعال شدن
    // زنده‌ی دکمه Next) با mandatory field های ثبت‌شده در سازنده (پسوند "*")
    // به‌صورت خودکار توسط خودِ QWizard مدیریت می‌شود - نیازی به override اینجا نیست.
    bool validatePage() override;

private:
    void updatePreview();

    QLineEdit *m_nameEdit = nullptr;
    QLineEdit *m_dirEdit = nullptr;
    QLabel *m_previewLabel = nullptr;
};

// صفحه ۲ ویزارد: انتخاب «بدون شماتیک» یا «ساخت شماتیک» + انتخاب یکی از
// قالب‌های استاندارد کاغذ (فقط وقتی «ساخت شماتیک» انتخاب شده باشد).
class SchematicSizePage : public QWizardPage
{
    Q_OBJECT

public:
    // preselectSchematic=true یعنی ورودی از مسیر «Template» در StartScreen بوده؛
    // در این حالت گزینه «Create Schematic» و اولین قالب از پیش انتخاب شده‌اند.
    explicit SchematicSizePage(bool preselectSchematic, QWidget *parent = nullptr);

    bool schematicEnabled() const { return m_createSchematic; }
    QString selectedTemplateName() const { return m_selectedTemplateName; }
    QSize selectedCanvasSize() const { return m_selectedCanvasSize; }

    bool isComplete() const override;

private:
    void selectMode(bool createSchematic);
    void selectTemplate(ClickableCard *card, const QString &name, QSize size);

    ClickableCard *m_noSchematicCard = nullptr;
    ClickableCard *m_createSchematicCard = nullptr;
    QWidget *m_templateGrid = nullptr;
    QList<ClickableCard *> m_templateCards;

    bool m_createSchematic = false;
    QString m_selectedTemplateName;
    QSize m_selectedCanvasSize;
};

// خودِ ویزارد - دو صفحه بالا را نگه می‌دارد و روی Finish (accept) فایل پروژه
// خالی را واقعاً روی دیسک می‌سازد؛ برای این کار یک CircuitScene موقت (فقط
// برای نگه‌داشتن اندازه بوم) ساخته و به همان ProjectSerializer که MainWindow
// برای Save/Load واقعی استفاده می‌کند می‌سپارد - تا منطق نوشتن فایل پروژه در
// دو جای مختلف کد تکرار/متفاوت نشود.
class NewProjectWizard : public QWizard
{
    Q_OBJECT

public:
    explicit NewProjectWizard(bool preferSchematic, QWidget *parent = nullptr);

    // فقط بعد از exec() == QDialog::Accepted معتبر است.
    QString createdProjectFilePath() const { return m_createdFilePath; }

    void accept() override;

private slots:
    void onHelpRequested();

private:
    ProjectInfoPage *m_infoPage = nullptr;
    SchematicSizePage *m_sizePage = nullptr;
    QString m_createdFilePath;
};

#endif // NEWPROJECTWIZARD_H
