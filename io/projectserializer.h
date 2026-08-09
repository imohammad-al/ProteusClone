#ifndef PROJECTSERIALIZER_H
#define PROJECTSERIALIZER_H

#include <QString>

class CircuitScene;

// مسئولیت این کلاس فقط و فقط ذخیره و بازیابی یک CircuitScene روی دیسک است
// (بخش ۱۰.۱ مستند پروژه). این کلاس هیچ منطق UI یا شبیه‌سازی ندارد؛
// تنها با API عمومی CircuitScene (components/wires/resetCircuit/connectPins)
// و Component::toJson/fromJson کار می‌کند.
class ProjectSerializer
{
public:
    // ذخیره مدار موجود در scene در فایل filePath (فرمت JSON).
    // در صورت شکست، false برمی‌گرداند و پیام خطا (در صورت غیر nullptr بودن) پر می‌شود.
    static bool save(CircuitScene *scene, const QString &filePath, QString *errorMessage = nullptr);

    // پاک کردن مدار فعلی scene و بازسازی آن از فایل filePath.
    static bool load(CircuitScene *scene, const QString &filePath, QString *errorMessage = nullptr);

private:
    ProjectSerializer() = delete; // این کلاس فقط شامل توابع استاتیک است
};

#endif // PROJECTSERIALIZER_H
