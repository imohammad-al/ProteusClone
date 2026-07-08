#include "dynamicpropertiesdialog.h"

DynamicPropertiesDialog::DynamicPropertiesDialog(const QMap<QString, ComponentProperty>& props, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("تنظیمات قطعه");
    auto* layout = new QFormLayout(this);

    // ساخت خودکار فیلدها بر اساس ویژگی‌های قطعه
    for (auto it = props.begin(); it != props.end(); ++it) {
        QString key = it.key();
        ComponentProperty prop = it.value();

        if (prop.type == "string") {
            auto* lineEdit = new QLineEdit(prop.value.toString(), this);
            layout->addRow(prop.label, lineEdit);
            m_widgets[key] = lineEdit;
        } else if (prop.type == "double") {
            auto* spinBox = new QDoubleSpinBox(this);
            spinBox->setMaximum(1000000000); // تا ۱ گیگا
            spinBox->setValue(prop.value.toDouble());
            layout->addRow(prop.label + " (" + prop.unit + ")", spinBox);
            m_widgets[key] = spinBox;
        }
    }

    // اضافه کردن دکمه‌های تایید و انصراف به پایین پنجره
    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addRow(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

// این تابع مقادیر جدیدی که کاربر تایپ کرده را پس می‌دهد
QMap<QString, QVariant> DynamicPropertiesDialog::getNewValues() const {
    QMap<QString, QVariant> results;
    for (auto it = m_widgets.begin(); it != m_widgets.end(); ++it) {
        if (auto* lineEdit = qobject_cast<QLineEdit*>(it.value())) {
            results[it.key()] = lineEdit->text();
        } else if (auto* spinBox = qobject_cast<QDoubleSpinBox*>(it.value())) {
            results[it.key()] = spinBox->value();
        }
    }
    return results;
}