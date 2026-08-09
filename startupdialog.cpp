#include "startupdialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QLabel>
#include <QFileDialog>
#include <QFrame>

StartupDialog::StartupDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Proteus Clone"));
    setModal(true);

    auto *mainLayout = new QVBoxLayout(this);

    auto *titleLabel = new QLabel(tr("<h2>Proteus Clone</h2>"), this);
    mainLayout->addWidget(titleLabel);

    // --- بخش پروژه جدید ---
    auto *newProjectFrame = new QFrame(this);
    newProjectFrame->setFrameShape(QFrame::StyledPanel);
    auto *newProjectLayout = new QVBoxLayout(newProjectFrame);

    newProjectLayout->addWidget(new QLabel(tr("<b>New Project</b>"), this));

    auto *sizeForm = new QFormLayout();

    m_widthSpin = new QSpinBox(this);
    m_widthSpin->setRange(500, 20000);
    m_widthSpin->setValue(3000);
    m_widthSpin->setSuffix(" px");

    m_heightSpin = new QSpinBox(this);
    m_heightSpin->setRange(500, 20000);
    m_heightSpin->setValue(3000);
    m_heightSpin->setSuffix(" px");

    sizeForm->addRow(tr("Canvas Width:"), m_widthSpin);
    sizeForm->addRow(tr("Canvas Height:"), m_heightSpin);
    newProjectLayout->addLayout(sizeForm);

    auto *newProjectButton = new QPushButton(tr("Start New Project"), this);
    connect(newProjectButton, &QPushButton::clicked, this, &StartupDialog::onNewProjectClicked);
    newProjectLayout->addWidget(newProjectButton);

    mainLayout->addWidget(newProjectFrame);

    // --- بخش باز کردن پروژه موجود ---
    auto *openButton = new QPushButton(tr("Open Existing Project..."), this);
    connect(openButton, &QPushButton::clicked, this, &StartupDialog::onOpenExistingClicked);
    mainLayout->addWidget(openButton);

    resize(380, 300);
}

QSize StartupDialog::chosenCanvasSize() const
{
    return QSize(m_widthSpin->value(), m_heightSpin->value());
}

void StartupDialog::onNewProjectClicked()
{
    m_choice = Choice::NewProject;
    accept();
}

void StartupDialog::onOpenExistingClicked()
{
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Open Project"), QString(),
        tr("Proteus Clone Project (*.pcproj *.json)"));

    if (path.isEmpty())
        return; // کاربر منصرف شد - دیالوگ باز می‌ماند تا انتخاب دیگری بکند

    m_filePath = path;
    m_choice = Choice::OpenExisting;
    accept();
}
