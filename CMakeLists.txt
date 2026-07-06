cmake_minimum_required(VERSION 3.19)
project(ProteusClone LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(ProteusClone
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    resistor.h resistor.cpp
    circuitscene.h circuitscene.cpp
    componentfactory.h componentfactory.cpp
    component.h component.cpp
    pin.h pin.cpp
    wire.h wire.cpp
    circuit.h circuit.cpp
    toolmanager.h toolmanager.cpp
    componentinfo.h
    librarymanager.h librarymanager.cpp
    twoterminalcomponent.h
    twoterminalcomponent.cpp
    commands/addcomponentcommand.h
    commands/addcomponentcommand.cpp
    commands/commandbase.h
    commands/commandbase.cpp
    commands/deletecomponentcommand.h commands/deletecomponentcommand.cpp
    commands/rotatecommand.h commands/rotatecommand.cpp
    commands/movecommand.h commands/movecommand.cpp
    node.h node.cpp
    junction.h junction.cpp
    wiresegment.h
    net.h net.cpp
    commands/addwirecommand.h commands/addwirecommand.cpp

)

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

qt_generate_deploy_app_script(
    TARGET ProteusClone
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
