include("/home/ryabi/my_apps/qt/getMusicApp/build/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/getMusicApp-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase")

qt6_deploy_runtime_dependencies(
    EXECUTABLE "/home/ryabi/my_apps/qt/getMusicApp/build/getMusicApp"
    GENERATE_QT_CONF
)
