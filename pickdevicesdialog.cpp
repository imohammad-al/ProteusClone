#include "pickdevicesdialog.h"
#include "componentpreviewwidget.h"
#include "librarymanager.h"
#include "componentinfo.h"

#include <QLineEdit>
#include <QCheckBox>
#include <QListWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QRegularExpression>
#include <QSet>

PickDevicesDialog::PickDevicesDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Pick Devices"));
    resize(760, 420);

    // ---------------- ستون چپ: جستجو + دسته‌بندی (بخش ۳.۱ و ۳.۲ مستند) ----------------
    m_keywordsEdit = new QLineEdit(this);
    m_matchWholeWordsCheck = new QCheckBox(tr("Match whole words"), this);
    m_categoryList = new QListWidget(this);
    m_categoryList->setMaximumWidth(170);

    auto *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(new QLabel(tr("Keywords:"), this));
    leftLayout->addWidget(m_keywordsEdit);
    leftLayout->addWidget(m_matchWholeWordsCheck);
    leftLayout->addWidget(new QLabel(tr("Category:"), this));
    leftLayout->addWidget(m_categoryList, 1);

    // ---------------- ستون وسط: نتایج (بخش ۳.۲ مستند) ----------------
    m_resultStatusLabel = new QLabel(this);
    m_resultsTable = new QTableWidget(0, 3, this);
    m_resultsTable->setHorizontalHeaderLabels({tr("Device"), tr("Category"), tr("Description")});
    m_resultsTable->horizontalHeader()->setStretchLastSection(true);
    m_resultsTable->verticalHeader()->setVisible(false);
    m_resultsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_resultsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_resultsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto *middleLayout = new QVBoxLayout;
    middleLayout->addWidget(m_resultStatusLabel);
    middleLayout->addWidget(m_resultsTable, 1);

    // ---------------- ستون راست: پیش‌نمایش زنده (بخش ۳.۳ مستند) ----------------
    m_previewCaption = new QLabel(this);
    m_previewWidget = new ComponentPreviewWidget(this);
    m_previewWidget->setMinimumWidth(180);

    auto *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(new QLabel(tr("Preview"), this));
    rightLayout->addWidget(m_previewWidget, 1);
    rightLayout->addWidget(m_previewCaption);

    auto *contentLayout = new QHBoxLayout;
    contentLayout->addLayout(leftLayout);
    contentLayout->addLayout(middleLayout, 1);
    contentLayout->addLayout(rightLayout);

    // ---------------- دکمه‌های پایین ----------------
    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &PickDevicesDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &PickDevicesDialog::reject);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(contentLayout, 1);
    mainLayout->addWidget(buttonBox);

    populateCategories();
    refreshResults();

    connect(m_keywordsEdit, &QLineEdit::textChanged, this, &PickDevicesDialog::refreshResults);
    connect(m_matchWholeWordsCheck, &QCheckBox::toggled, this, &PickDevicesDialog::refreshResults);
    connect(m_categoryList, &QListWidget::currentItemChanged, this, &PickDevicesDialog::onCategorySelectionChanged);
    connect(m_resultsTable, &QTableWidget::itemSelectionChanged, this, &PickDevicesDialog::onResultSelectionChanged);
    connect(m_resultsTable, &QTableWidget::cellDoubleClicked, this, &PickDevicesDialog::onResultActivated);

    m_keywordsEdit->setFocus();
}

void PickDevicesDialog::populateCategories()
{
    m_categoryList->clear();

    auto *allItem = new QListWidgetItem(tr("(All Categories)"), m_categoryList);
    allItem->setData(Qt::UserRole, QString()); // رشته خالی یعنی فیلتر دسته‌بندی خاموش است

    QSet<QString> seen;
    QStringList categories;
    for (const ComponentInfo &info : LibraryManager::components()) {
        if (!seen.contains(info.category)) {
            seen.insert(info.category);
            categories << info.category;
        }
    }
    categories.sort(Qt::CaseInsensitive);

    for (const QString &category : categories) {
        auto *item = new QListWidgetItem(category, m_categoryList);
        item->setData(Qt::UserRole, category);
    }

    m_categoryList->setCurrentItem(allItem);
}

void PickDevicesDialog::onCategorySelectionChanged()
{
    refreshResults();
}

void PickDevicesDialog::refreshResults()
{
    m_resultsTable->setRowCount(0);

    const QString needle = m_keywordsEdit->text().trimmed();
    const bool wholeWords = m_matchWholeWordsCheck->isChecked();

    QString selectedCategory;
    if (QListWidgetItem *catItem = m_categoryList->currentItem())
        selectedCategory = catItem->data(Qt::UserRole).toString();

    QRegularExpression wordRegex;
    if (wholeWords && !needle.isEmpty()) {
        wordRegex = QRegularExpression(
            QStringLiteral("\\b%1\\b").arg(QRegularExpression::escape(needle)),
            QRegularExpression::CaseInsensitiveOption);
    }

    auto matchesText = [&](const QString &text) -> bool {
        if (needle.isEmpty())
            return true;
        if (wholeWords)
            return wordRegex.match(text).hasMatch();
        return text.contains(needle, Qt::CaseInsensitive);
    };

    int row = 0;
    for (const ComponentInfo &info : LibraryManager::components()) {
        if (!selectedCategory.isEmpty() && info.category != selectedCategory)
            continue;
        // بخش ۳.۲ مستند: جستجو باید هم روی نام قطعه و هم روی دسته‌بندی کار کند
        if (!matchesText(info.name) && !matchesText(info.category))
            continue;

        m_resultsTable->insertRow(row);
        auto *nameItem = new QTableWidgetItem(info.name);
        auto *catItem = new QTableWidgetItem(info.category);
        auto *descItem = new QTableWidgetItem(info.description);
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        catItem->setFlags(catItem->flags() & ~Qt::ItemIsEditable);
        descItem->setFlags(descItem->flags() & ~Qt::ItemIsEditable);
        m_resultsTable->setItem(row, 0, nameItem);
        m_resultsTable->setItem(row, 1, catItem);
        m_resultsTable->setItem(row, 2, descItem);
        ++row;
    }

    if (row == 0) {
        // بخش ۳.۲ مستند: "اگر قطعه‌ای پیدا نشد، یک پیام مناسب به کاربر نمایش دهید"
        m_resultStatusLabel->setText(tr("No matching parts found."));
        m_previewWidget->clear();
        m_previewCaption->clear();
    } else {
        m_resultStatusLabel->setText(tr("Showing %1 result(s)").arg(row));
        m_resultsTable->selectRow(0);
    }
}

void PickDevicesDialog::onResultSelectionChanged()
{
    const QList<QTableWidgetItem *> selected = m_resultsTable->selectedItems();
    if (selected.isEmpty()) {
        m_previewWidget->clear();
        m_previewCaption->clear();
        return;
    }
    const int row = selected.first()->row();
    const QString name = m_resultsTable->item(row, 0)->text();
    m_previewWidget->setComponentType(name);
    m_previewCaption->setText(name);
}

void PickDevicesDialog::onResultActivated(int row, int)
{
    if (row < 0 || !m_resultsTable->item(row, 0))
        return;
    m_selectedName = m_resultsTable->item(row, 0)->text();
    QDialog::accept();
}

void PickDevicesDialog::accept()
{
    const QList<QTableWidgetItem *> selected = m_resultsTable->selectedItems();
    if (selected.isEmpty())
        return; // چیزی انتخاب نشده - دیالوگ را نبند (دکمه OK بی‌اثر می‌ماند)
    m_selectedName = m_resultsTable->item(selected.first()->row(), 0)->text();
    QDialog::accept();
}
