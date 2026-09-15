cmake_minimum_required(VERSION 3.19)
project(getMusicApp LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network LinguistTools)


include(FetchContent)

set(QUAZIP_ENABLE_QTEXTCODEC OFF CACHE BOOL "Disable Core5Compat check" FORCE)

FetchContent_Declare(
    quazip
    GIT_REPOSITORY "https://github.com/stachenov/quazip.git"
    GIT_TAG        "v1.7.0"
)

FetchContent_MakeAvailable(quazip)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

qt_add_executable(getMusicApp
    WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/ui/mainwindow.cpp
    src/ui/logView.cpp
    src/core/downloadManager.cpp
    src/ui/settingDialog.cpp
    src/core/downloadManager.h
    src/ui/logView.h
    src/ui/mainwindow.h
    src/ui/settingDialog.h
)

qt_add_translations(getMusicApp
    TS_FILES 
        translations/translate_ru.ts
    RESOURCE_PREFIX
        "/i18n"
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