#ifndef SIMULATIONLOGGER_H
#define SIMULATIONLOGGER_H

#include <QObject>
#include <QString>

// سطح اهمیت پیام - برای رنگ‌بندی و اولویت‌بندی در پنجره گزارشات (بخش ۱۱.۳ مستند پروژه)
enum class LogLevel {
    Info,
    Warning,
    Error
};

// این کلاس تنها مسئولیتش انتشار پیام‌های لاگ است (الگوی Publisher/Subscriber).
// DesignRuleChecker و در آینده موتور شبیه‌سازی، MCU و ... فقط با این کلاس صحبت می‌کنند
// و هیچ اطلاعی از وجود MainWindow یا هر ویجت دیگری ندارند؛ این جدا بودن UI از منطق
// همان چیزی است که مستند پروژه هم روی آن تاکید کرده (Backend/Frontend).
class SimulationLogger : public QObject
{
    Q_OBJECT

public:
    static SimulationLogger &instance();

    void log(LogLevel level, const QString &message);
    void clear();

signals:
    void messageLogged(LogLevel level, const QString &message);
    void cleared();

private:
    explicit SimulationLogger(QObject *parent = nullptr);
    SimulationLogger(const SimulationLogger &) = delete;
    SimulationLogger &operator=(const SimulationLogger &) = delete;
};

#endif // SIMULATIONLOGGER_H
