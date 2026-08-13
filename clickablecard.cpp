#include "clickablecard.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QStyle>

ClickableCard::ClickableCard(const QString &title, const QString &subtitle, QWidget *parent)
    : QFrame(parent)
{
    // objectName روی "clickableCard" ثابت است تا selector مشترک styleSheet()
    // (که با #clickableCard نوشته شده) روی همه نمونه‌ها اعمال شود.
    setObjectName(QStringLiteral("clickableCard"));
    setCursor(Qt::PointingHandCursor);
    setFrameShape(QFrame::NoFrame);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(14, 14, 14, 12);
    layout->setSpacing(6);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setObjectName(QStringLiteral("cardIcon"));
    layout->addWidget(m_iconLabel);

    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setObjectName(QStringLiteral("cardTitle"));
    m_titleLabel->setWordWrap(true);
    layout->addWidget(m_titleLabel);

    m_subtitleLabel = new QLabel(subtitle, this);
    m_subtitleLabel->setObjectName(QStringLiteral("cardSubtitle"));
    m_subtitleLabel->setWordWrap(true);
    m_subtitleLabel->setVisible(!subtitle.isEmpty());
    layout->addWidget(m_subtitleLabel);

    layout->addStretch();

    refreshVisualState();
}

void ClickableCard::setIconPixmap(const QPixmap &pm)
{
    m_iconLabel->setPixmap(pm);
    m_iconLabel->setFixedSize(pm.size());
}

void ClickableCard::setCheckable(bool checkable)
{
    m_checkable = checkable;
}

void ClickableCard::setChecked(bool checked)
{
    if (!m_checkable || m_checked == checked)
        return;

    m_checked = checked;
    refreshVisualState();
    emit toggled(m_checked);
}

void ClickableCard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isEnabled()) {
        m_pressed = true;
        refreshVisualState();
    }
    QFrame::mousePressEvent(event);
}

void ClickableCard::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_pressed) {
        m_pressed = false;
        if (isEnabled() && rect().contains(event->pos())) {
            if (m_checkable)
                setChecked(true); // انحصاری‌بودن (Exclusive) با clicked() توسط صفحه‌ی مالک مدیریت می‌شود
            emit clicked();
        }
        refreshVisualState();
    }
    QFrame::mouseReleaseEvent(event);
}

void ClickableCard::enterEvent(QEnterEvent *event)
{
    // توجه: امضای این تابع در Qt6 عوض شده (QEnterEvent* به‌جای QEvent* قدیمیِ Qt5).
    m_hovered = true;
    refreshVisualState();
    QFrame::enterEvent(event);
}

void ClickableCard::leaveEvent(QEvent *event)
{
    m_hovered = false;
    m_pressed = false;
    refreshVisualState();
    QFrame::leaveEvent(event);
}

void ClickableCard::refreshVisualState()
{
    // به‌جای تکیه به شبه‌کلاس‌های استاندارد QSS (:hover و ...) که برای QFrame
    // بدون WA_Hover همیشه قابل‌اعتماد نیستند، وضعیت را خودمان به‌صورت Dynamic
    // Property ثبت می‌کنیم و style را دستی Repolish می‌کنیم - قابل پیش‌بینی‌تر
    // و مستقل از رفتار داخلی هاور در استایل‌های مختلف پلتفرم.
    setProperty("hovered", m_hovered);
    setProperty("pressed", m_pressed);
    setProperty("checked", m_checked);
    style()->unpolish(this);
    style()->polish(this);
    update();
}

QString ClickableCard::cardStyleSheet()
{
    return QStringLiteral(
        "#clickableCard {"
        "   background-color: #FFFFFF;"
        "   border: 1px solid #E1E5EC;"
        "   border-radius: 10px;"
        "}"
        "#clickableCard[hovered=\"true\"] {"
        "   border: 1px solid #286EC8;"
        "   background-color: #F2F7FE;"
        "}"
        "#clickableCard[pressed=\"true\"] {"
        "   background-color: #E7F0FC;"
        "}"
        "#clickableCard[checked=\"true\"] {"
        "   border: 2px solid #286EC8;"
        "   background-color: #EAF2FF;"
        "}"
        "#clickableCard:disabled {"
        "   background-color: #F4F6F9;"
        "   border: 1px dashed #D8DCE3;"
        "}"
        "#clickableCard:disabled QLabel {"
        "   color: #B0B6C0;"
        "}"
        "#cardTitle {"
        "   font-size: 14px;"
        "   font-weight: 600;"
        "   color: #22262E;"
        "}"
        "#cardSubtitle {"
        "   font-size: 11px;"
        "   color: #7A828F;"
        "}"
    );
}
