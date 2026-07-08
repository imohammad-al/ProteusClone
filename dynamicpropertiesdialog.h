#include <QDialog>
#include <QMap>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include "component.h" // برای دسترسی به ساختار ComponentProperty

class DynamicPropertiesDialog : public QDialog {
    Q_OBJECT
public:
    DynamicPropertiesDialog(const QMap<QString, ComponentProperty>& props, QWidget *parent = nullptr);
    QMap<QString, QVariant> getNewValues() const;

private:
    QMap<QString, QWidget*> m_widgets; // برای ذخیره پوینتر ویجت‌ها
};