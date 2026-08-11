cmake_minimum_required(VERSION 3.19)
project(ProteusClone LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

# لیست مشترک منابع "هسته" (همه‌چیز به‌جز main.cpp/mainwindow.* که فقط GUI
# اصلی نیازشون داره) - فاز ۱۶: این لیست بین هدف اصلی ProteusClone و هدف تست
# analog_solver_tests به اشتراک گذاشته می‌شه تا از واگرایی (فراموش‌کردن
# اضافه‌کردن فایل جدید به یکی از دو تارگت) جلوگیری بشه.
set(PROTEUSCLONE_CORE_SOURCES
    circuitgraphicsview.h circuitgraphicsview.cpp
    circuitscene.h circuitscene.cpp
    componentfactory.h componentfactory.cpp
    component.h component.cpp
    pin.h pin.cpp
    wire.h wire.cpp
    circuit.h circuit.cpp
    toolmanager.h toolmanager.cpp
    componentinfo.h
    librarymanager.h librarymanager.cpp

    commands/addcomponentcommand.h
    commands/addcomponentcommand.cpp
    commands/commandbase.h
    commands/commandbase.cpp
    commands/deletecomponentcommand.h commands/deletecomponentcommand.cpp
    commands/deletewirecommand.h commands/deletewirecommand.cpp
    commands/rotatecommand.h commands/rotatecommand.cpp
    commands/mirrorcommand.h commands/mirrorcommand.cpp
    commands/movecommand.h commands/movecommand.cpp
    node.h node.cpp
    junction.h junction.cpp
    wiresegment.h
    net.h net.cpp
    commands/addwirecommand.h commands/addwirecommand.cpp
    wirecontroller.h wirecontroller.cpp
    wireitem.h wireitem.cpp
    dynamicpropertiesdialog.h dynamicpropertiesdialog.cpp

    passives.h passives.cpp
    sources.h sources.cpp
    semiconductors.h semiconductors.cpp
    logicgates.h logicgates.cpp

    digitallogic.h
    digitalcomponent.h digitalcomponent.cpp
    interactivecomponents.h interactivecomponents.cpp
    measurementtools.h measurementtools.cpp
    mcu.h mcu.cpp
    lcd.h lcd.cpp
    keypad.h keypad.cpp
    adc.h adc.cpp
    dac.h dac.cpp
    externalmemory.h externalmemory.cpp

    io/projectserializer.h io/projectserializer.cpp
    sim/designrulechecker.h sim/designrulechecker.cpp
    sim/simulationlogger.h sim/simulationlogger.cpp
    sim/simulationengine.h sim/simulationengine.cpp
    sim/analogsolver.h sim/analogsolver.cpp
)

qt_add_executable(ProteusClone
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    startupdialog.h startupdialog.cpp
    ${PROTEUSCLONE_CORE_SOURCES}
)

target_link_libraries(ProteusClone
    PRIVATE
        Qt::Core
        Qt::Widgets
)

# باگ واقعی که با اولین کامپایل واقعی این پروژه کشف شد (فاز ۱۶): بدون این خط،
# هدرهای تولیدشده توسط uic (مثل ui_mainwindow.h که include "circuitgraphicsview.h"
# داره) نمی‌تونن هدرهای هم‌سطحشون رو پیدا کنن، چون ریشه‌ی پروژه اصلاً توی
# مسیر include کامپایلر نبود. تا حالا این باگ دیده نشده بود چون تمام فازهای
# قبلی (۰ تا ۱۵) بدون کامپایلر واقعی و فقط با بازبینی دستی کد تأیید شده بودن.
target_include_directories(ProteusClone PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS ProteusClone
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET ProteusClone
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})

# --- تست‌های خودکار (فاز ۱۶) ---
# هارنس مستقل خارج از GUI که مدارهای واقعی (ترانزیستور/دیود) را با همان
# کلاس‌های تولید می‌سازه و نتیجه‌ی AnalogSolver را با مقادیر مرجع مستقل (حل‌شده
# در پایتون) مقایسه می‌کنه - جزئیات در tests/test_analog_solver.cpp. اولین بار
# در فاز ۱۶ که یک کامپایلر واقعی در دسترس بود نوشته شد و همون‌جا یک باگ واقعی
# در استمپ‌گذاری ترانزیستور رو کشف کرد؛ برای جلوگیری از رگرسیون در فازهای بعدی
# نگه داشته شده. با -DPROTEUSCLONE_BUILD_TESTS=OFF قابل غیرفعال‌سازیه.
option(PROTEUSCLONE_BUILD_TESTS "Build the analog solver test harness (tests/)" ON)
if(PROTEUSCLONE_BUILD_TESTS)
    qt_add_executable(analog_solver_tests
        tests/test_analog_solver.cpp
        ${PROTEUSCLONE_CORE_SOURCES}
    )
    target_include_directories(analog_solver_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(analog_solver_tests PRIVATE Qt::Core Qt::Widgets)
endif()
