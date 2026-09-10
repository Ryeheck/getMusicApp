cmake_minimum_required(VERSION 3.19)
project(getMusicApp LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network)


include(FetchContent)

set(QUAZIP_ENABLE_QTEXTCODEC OFF CACHE BOOL "Disable Core5Compat check" FORCE)

FetchContent_Declare(
    quazip
    GIT_REPOSITORY "https://github.com/stachenov/quazip.git"
    GIT_TAG        "v1.7.0"
)

FetchContent_MakeAvailable(quazip)

qt_standard_project_setup()

qt_add_executable(getMusicApp
    WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/mainwindow.cpp
    src/logView.cpp
    src/downloadManager.cpp
    src/settingDialog.cpp
    src/downloadManager.h
    src/logView.h
    src/mainwindow.h
    src/settingDialog.h
)

target_link_libraries(getMusicApp
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Network
        QuaZip::QuaZip
)

include(GNUInstallDirs)

install(TARGETS getMusicApp
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET getMusicApp
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})