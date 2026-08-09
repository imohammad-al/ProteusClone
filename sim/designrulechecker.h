#ifndef DESIGNRULECHECKER_H
#define DESIGNRULECHECKER_H

#include <QList>
#include <QString>

class CircuitScene;

enum class DrcSeverity {
    Info,
    Warning,
    Error
};

struct DrcIssue {
    DrcSeverity severity;
    QString message;
};

// این کلاس فقط مسئول بررسی قوانین طراحی مدار است (بخش ۱۱ مستند پروژه):
//  - پایه‌های رها شده (Floating Pins) → همیشه قابل بررسی، مستقل از موتور شبیه‌سازی
//  - اتصال کوتاه ساده‌شده: چند منبع ولتاژ (Ground/DC Source) که مستقیماً روی یک گره قرار گرفته‌اند
//
// نکته مهم: تشخیص کامل اتصال کوتاه طبق تعریف دقیق پروپوزال (دو سیگنال دیجیتال متضاد
// روی یک سیم) نیازمند موتور شبیه‌سازی منطقی (تعیین HIGH/LOW/Undefined هر پایه) است که
// هنوز پیاده نشده؛ این نسخه یک بررسی توپولوژیک (بدون نیاز به اجرای شبیه‌سازی) انجام می‌دهد
// و در فاز شبیه‌سازی باید با بررسی سطح‌منطقی واقعی تکمیل شود.
class DesignRuleChecker
{
public:
    static QList<DrcIssue> check(CircuitScene *scene);

private:
    DesignRuleChecker() = delete;
};

#endif // DESIGNRULECHECKER_H
