#include "designrulechecker.h"
#include "../circuitscene.h"
#include "../component.h"
#include "../pin.h"
#include "../node.h"

QList<DrcIssue> DesignRuleChecker::check(CircuitScene *scene)
{
    QList<DrcIssue> issues;
    if (!scene)
        return issues;

    // --- ۱. پایه‌های رها شده (بخش ۱۱.۲) ---
    // هر پایه‌ای که هیچ سیمی به آن وصل نشده، رها شده (Floating) است.
    const QList<Component*> comps = scene->components();
    for (Component *c : comps) {
        const QString label = c->name().isEmpty() ? c->componentType() : c->name();
        for (Pin *p : c->pins()) {
            if (p && p->wires().isEmpty()) {
                issues.append({DrcSeverity::Warning,
                    QObject::tr("پایه رها شده (Floating): قطعه \"%1\" (%2)")
                        .arg(label, c->componentType())});
            }
        }
    }

    // --- ۲. اتصال کوتاه ساده‌شده (بخش ۱۱.۱) ---
    // اگر بیش از یک منبع ولتاژ ثابت (Ground یا DC Source) مستقیماً روی یک گره قرار گرفته باشند،
    // از نظر توپولوژیک نمی‌توانند هر دو مقدار خودشان را همزمان به آن گره تحمیل کنند.
    const QList<Node*> nodeList = scene->nodes();
    for (Node *node : nodeList) {
        QStringList sourceLabels;
        const QList<Pin*> nodePins = node->pins();
        for (Pin *p : nodePins) {
            Component *owner = p ? dynamic_cast<Component*>(p->parentItem()) : nullptr;
            if (!owner) continue;
            if (owner->componentType() == "Ground" || owner->componentType() == "DC Source") {
                sourceLabels << (owner->name().isEmpty() ? owner->componentType() : owner->name());
            }
        }
        if (sourceLabels.size() > 1) {
            issues.append({DrcSeverity::Error,
                QObject::tr("احتمال اتصال کوتاه: منابع ولتاژ (%1) مستقیماً به یک گره متصل شده‌اند")
                    .arg(sourceLabels.join(", "))});
        }
    }

    // طبق بخش ۱۱.۳ مستند: اگر هیچ خطایی وجود نداشت، این موضوع هم باید صراحتاً اعلام شود.
    if (issues.isEmpty()) {
        issues.append({DrcSeverity::Info, QObject::tr("هیچ خطایی در مدار یافت نشد.")});
    }

    return issues;
}
