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

        // نکته: Component::setProperty() فقط displayName را پر می‌کند (نه label)،
        // و value.typeName() برای رشته‌ها "QString" برمی‌گرداند نه "string".
        // این دو نکته قبلاً باعث می‌شد لیبل‌ها خالی بمونن و فیلدهای متنی
        // (مثل model دیود/ترانزیستور) اصلاً ساخته نشن.
        QString label = prop.displayName.isEmpty() ? key : prop.displayName;

        if (prop.type == "QString") {
            auto* lineEdit = new QLineEdit(prop.value.toString(), this);
            layout->addRow(label, lineEdit);
            m_widgets[key] = lineEdit;
        } else if (prop.type == "double") {
            auto* spinBox = new QDoubleSpinBox(this);
            spinBox->setMaximum(1000000000); // تا ۱ گیگا
            spinBox->setValue(prop.value.toDouble());
            QString rowLabel = prop.unit.isEmpty() ? label : label + " (" + prop.unit + ")";
            layout->addRow(rowLabel, spinBox);
            m_widgets[key] = spinBox;
        } else if (prop.type == "bool") {
            auto* checkBox = new QCheckBox(this);
            checkBox->setChecked(prop.value.toBool());
            layout->addRow(label, checkBox);
            m_widgets[key] = checkBox;
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
        } else if (auto* checkBox = qobject_cast<QCheckBox*>(it.value())) {
            results[it.key()] = checkBox->isChecked();
        }
    }
    return results;
}