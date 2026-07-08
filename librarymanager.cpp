#include "librarymanager.h"
#include "resistor.h"
#include "capacitor.h" // <-- ۱. این خط را اضافه کردیم تا فایل خازن را بشناسد
#include "diode.h"
#include "dcsource.h"

QList<ComponentInfo> LibraryManager::m_components;

void LibraryManager::initialize()
{
    if(!m_components.isEmpty())
        return;

    // --- ثبت مقاومت (کد قبلی شما بدون تغییر) ---
    ComponentInfo resistor;
    resistor.name = "Resistor";
    resistor.category = "Passive";
    resistor.description = "Electrical Resistor";
    resistor.creator = []()
    {
        return new Resistor;
    };
    m_components.append(resistor);

    // --- ۲. ثبت خازن (این بخش جدید را اضافه کنید) ---
    ComponentInfo capacitor;
    capacitor.name = "Capacitor";
    capacitor.category = "Passive"; // چون خازن هم یک قطعه پسیو یا غیرفعال است
    capacitor.description = "Electrical Capacitor";
    capacitor.creator = []()
    {
        return new Capacitor; // این تابع وقتی کاربر روی خازن کلیک کند، یک خازن جدید می‌سازد
    };
    m_components.append(capacitor);

    // ثبت دیود
    ComponentInfo diode;
    diode.name = "Diode";
    diode.category = "Discrete"; // دسته‌بندی قطعات مجزا یا نیمه‌هادی
    diode.description = "Semiconductor Diode";
    diode.creator = []() { return new Diode; };
    m_components.append(diode);

    // ثبت منبع ولتاژ DC
    ComponentInfo dcSource;
    dcSource.name = "DC Source";
    dcSource.category = "Sources"; // دسته‌بندی منابع تغذیه
    dcSource.description = "Direct Current Voltage Source";
    dcSource.creator = []() { return new DCSource; };
    m_components.append(dcSource);
}

QList<ComponentInfo> LibraryManager::components()
{
    return m_components;
}