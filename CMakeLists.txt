cmake_minimum_required(VERSION 3.19)
project(ProteusClone LANGUAGES CXX)

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

# --- منابع مشترک «موتور» برنامه (همه چیز به‌جز main.cpp) ---
# این لیست هم توسط اجرایی اصلی GUI و هم توسط اجرایی تست مستقل رگرسیون آنالوگ
# (تست‌های مربوط به AnalogSolver) استفاده می‌شود، تا فایل‌های سورس در دو جای
# مختلف تکرار نشوند و از هماهنگ‌ماندن خودکار دو هدف بیلد مطمئن باشیم.
set(PROTEUS_ENGINE_SOURCES
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    startscreen.h startscreen.cpp
    newprojectwizard.h newprojectwizard.cpp
    clickablecard.h clickablecard.cpp
    logofactory.h logofactory.cpp
    recentprojectsmanager.h recentprojectsmanager.cpp
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
    iconfactory.h iconfactory.cpp
    componentpreviewwidget.h componentpreviewwidget.cpp
    pickdevicesdialog.h pickdevicesdialog.cpp

    io/projectserializer.h io/projectserializer.cpp
    sim/designrulechecker.h sim/designrulechecker.cpp
    sim/simulationlogger.h sim/simulationlogger.cpp
    sim/simulationengine.h sim/simulationengine.cpp
    sim/analogsolver.h sim/analogsolver.cpp
)

qt_add_executable(ProteusClone
    WIN32 MACOSX_BUNDLE
    main.cpp
    ${PROTEUS_ENGINE_SOURCES}
)
target_include_directories(ProteusClone PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(ProteusClone
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS ProteusClone
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# --- اجرایی تست مستقل رگرسیون AnalogSolver (بدون GUI، خارج از MainWindow) ---
# دقیقاً از همان منابع «موتور» بالا استفاده می‌کند تا هیچ‌وقت از رفتار واقعی
# برنامه عقب نیفتد، فقط main.cpp با tests/test_analog_solver.cpp عوض می‌شود.
option(PROTEUS_BUILD_TESTS "ساخت اجرایی تست مستقل AnalogSolver" ON)
if(PROTEUS_BUILD_TESTS)
    qt_add_executable(ProteusCloneTests
        tests/test_analog_solver.cpp
        ${PROTEUS_ENGINE_SOURCES}
    )
    target_include_directories(ProteusCloneTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(ProteusCloneTests
        PRIVATE
            Qt::Core
            Qt::Widgets
    )
endif()

# qt_generate_deploy_app_script() با آرگومان OUTPUT_SCRIPT فقط در Qt 6.5+ پشتیبانی می‌شود؛
# روی Qt 6.4 (کف نسخه‌ای که بالا اعلام شده) خطای پیکربندی می‌دهد. این بخش صرفاً اسکریپت
# نصب/بسته‌بندی برنامه است و ربطی به منطق برنامه ندارد، پس فقط وقتی Qt 6.5+ موجود باشد اجرا می‌شود.
if(QT_VERSION VERSION_GREATER_EQUAL 6.5)
    qt_generate_deploy_app_script(
        TARGET ProteusClone
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${deploy_script})
endif()
