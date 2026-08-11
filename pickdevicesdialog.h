#ifndef PICKDEVICESDIALOG_H
#define PICKDEVICESDIALOG_H

#include <QDialog>

class QLineEdit;
class QCheckBox;
class QListWidget;
class QListWidgetItem;
class QTableWidget;
class QLabel;
class ComponentPreviewWidget;

// پنجره «Pick Devices» - بخش‌های ۳.۱ تا ۳.۳ مستند پروژه (دسته‌بندی ساختاریافته/
// جستجوی زنده روی نام و دسته/پیش‌نمایش شماتیک)، با همان چیدمان پنجره واقعی
// Pick Devices نرم‌افزار پروتئوس (طبق عکس مرجعی که کاربر فرستاد): جستجو و
// دسته‌ها در ستون چپ، نتایج در جدول وسط، پیش‌نمایش زنده در ستون راست.
//
// این دیالوگ جایگزین QInputDialog::getItem ساده‌ی قبلی شد (بخش ۴ مستند به آن
// اشاره می‌کرد که کاربر باید «محیط راحتی برای دسترسی به قطعات» داشته باشد، نه
// یک لیست متنی خشک).
class PickDevicesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PickDevicesDialog(QWidget *parent = nullptr);

    // نام دقیق componentType() قطعه انتخاب‌شده (برای مثال "Resistor")، یا رشته
    // خالی اگر کاربر با Cancel بسته باشد. فقط بعد از exec()==QDialog::Accepted معتبر است.
    QString selectedComponentName() const { return m_selectedName; }

private slots:
    void refreshResults();
    void onCategorySelectionChanged();
    void onResultSelectionChanged();
    void onResultActivated(int row, int column);

private:
    void populateCategories();
    void accept() override;

    QLineEdit *m_keywordsEdit = nullptr;
    QCheckBox *m_matchWholeWordsCheck = nullptr;
    QListWidget *m_categoryList = nullptr;
    QTableWidget *m_resultsTable = nullptr;
    ComponentPreviewWidget *m_previewWidget = nullptr;
    QLabel *m_previewCaption = nullptr;
    QLabel *m_resultStatusLabel = nullptr;

    QString m_selectedName;
};

#endif // PICKDEVICESDIALOG_H
