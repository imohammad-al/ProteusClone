include("A:/Projects/QTProjects/ProteusClone/ProteusClone/build/Desktop_x86_windows_msys_pe_64bit_Debug/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/ProteusClone-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase")

qt6_deploy_runtime_dependencies(
    EXECUTABLE A:/Projects/QTProjects/ProteusClone/ProteusClone/build/Desktop_x86_windows_msys_pe_64bit_Debug/ProteusClone.exe
    GENERATE_QT_CONF
)
