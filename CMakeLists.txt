cmake_minimum_required(VERSION 3.19)
project(getMusicApp LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

# add_executable(my_app main.cpp)


qt_standard_project_setup()

qt_add_executable(getMusicApp
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    Music/songs
    mainwindow.h
    mainwindow.ui
)

target_link_libraries(getMusicApp
    PRIVATE
        Qt::Core
        Qt::Widgets
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