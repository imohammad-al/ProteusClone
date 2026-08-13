#ifndef CLICKABLECARD_H
#define CLICKABLECARD_H

#include <QFrame>
#include <QString>
#include <QPixmap>

class QLabel;

// یک کارت کلیک‌پذیر (آیکون + عنوان درشت + زیرنویس کم‌رنگ اختیاری).
//
// چرا به‌جای QPushButton؟ چون QPushButton متن غنی/چندسطحی با استایل متفاوت
// برای عنوان و زیرنویس را پشتیبانی نمی‌کند. این ویجت با QFrame + Layout ساخته
// شده و رویدادهای ماوس (press/release/hover) را خودش مدیریت می‌کند تا هم
// حالت «فقط کلیک‌پذیر» (کارت‌های New Project/Open Project/Template در
// StartScreen) و هم حالت «قابل‌انتخاب مثل رادیو-باتن» (کارت‌های ویزارد پروژه
// جدید: No Schematic/Create Schematic و قالب‌های کاغذ) را پوشش بدهد.
//
// مدیریت انحصاری‌بودن (Exclusive - شبیه QButtonGroup) عمداً داخل خودِ این
// کلاس نیست؛ چون فقط دو محل مصرف با گروه‌بندی متفاوت وجود دارد (SchematicSizePage)
// و ساده‌تر است که خودِ آن صفحه با گوش‌دادن به clicked() این کار را انجام دهد.
class ClickableCard : public QFrame
{
    Q_OBJECT

public:
    explicit ClickableCard(const QString &title, const QString &subtitle = QString(),
                            QWidget *parent = nullptr);

    void setIconPixmap(const QPixmap &pm);

    void setCheckable(bool checkable);
    bool isCheckable() const { return m_checkable; }

    void setChecked(bool checked);
    bool isChecked() const { return m_checked; }

    // بخش مشترک QSS برای ظاهر این کارت‌ها (هاور/فشرده‌شدن/انتخاب/غیرفعال) - هم
    // StartScreen و هم NewProjectWizard این را به استایل‌شیت خودشان اضافه
    // می‌کنند تا این بخش دوبار نوشته نشود.
    // (به‌عمد "cardStyleSheet" نامیده شده، نه "styleSheet"، تا با متد به‌ارث‌رسیده
    // QWidget::styleSheet() - که یک getter نمونه‌ای/instance-level برای CSS
    // خودِ ویجت است - اشتباه/Hide نشود.)
    static QString cardStyleSheet();

signals:
    void clicked();
    void toggled(bool checked);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void refreshVisualState();

    QLabel *m_iconLabel = nullptr;
    QLabel *m_titleLabel = nullptr;
    QLabel *m_subtitleLabel = nullptr;

    bool m_checkable = false;
    bool m_checked = false;
    bool m_hovered = false;
    bool m_pressed = false;
};

#endif // CLICKABLECARD_H
