#ifndef COMPONENTPREVIEWWIDGET_H
#define COMPONENTPREVIEWWIDGET_H

#include <QWidget>

class Component;

// یک بوم کوچک که یک نمونه‌ی زنده از یک قطعه را نمایش می‌دهد - با فراخوانی مستقیم
// همان Component::paint() که خودِ قطعه روی بوم اصلی مدار استفاده می‌کند، پس این
// پیش‌نمایش همیشه دقیقاً با چیزی که کاربر بعد از قرار دادن قطعه روی مدار می‌بیند
// یکی است (نه یک آیکون دستیِ جداگانه که ممکن است از طرح واقعی قطعه عقب بماند).
// بخش ۳.۳ مستند پروژه («پیش‌نمایش شماتیک قطعه»).
class ComponentPreviewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ComponentPreviewWidget(QWidget *parent = nullptr);
    ~ComponentPreviewWidget() override;

    // typeName باید دقیقاً همان componentType() ثبت‌شده در LibraryManager باشد
    // (مثلاً "Resistor")؛ یک نمونه موقت از ComponentFactory می‌سازد که به هیچ
    // صحنه‌ای اضافه نمی‌شود، فقط برای رسم پیش‌نمایش نگه داشته می‌شود.
    void setComponentType(const QString &typeName);
    void clear();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Component *m_previewInstance = nullptr;
    QString m_typeName;
};

#endif // COMPONENTPREVIEWWIDGET_H
