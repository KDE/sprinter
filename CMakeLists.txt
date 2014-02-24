cmake_minimum_required(VERSION 2.8)
include (GenerateExportHeader)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH
        ${ECM_MODULE_PATH}
        ${ECM_KDE_MODULE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
        ${CMAKE_MODULE_PATH})

include(KDEInstallDirs)

find_package(Qt5Core REQUIRED)
find_package(Qt5Declarative REQUIRED)

set(CMAKE_AUTOMOC ON)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

add_definitions("-Wall -std=c++0x")

# the main library
add_subdirectory(sprinter)

### Test app - temporary, will be removed at some point
set(sprinter_SRCS
    test/main.cpp
)
add_executable(sprintertest ${sprinter_SRCS})
qt5_use_modules(sprintertest Widgets Network)
target_link_libraries(sprintertest sprinter)
install(TARGETS sprintertest DESTINATION bin)

install(FILES ${sprinterlib_HEADERS} DESTINATION include/sprinter/)

### QML plugins
set(qmlsprinterplugin_SRCS
    qml/sprinterplugin.cpp
)

add_library(sprinterplugin SHARED ${qmlsprinterplugin_SRCS})
qt5_use_modules(sprinterplugin Widgets Network Declarative)
target_link_libraries(sprinterplugin sprinter)

install(TARGETS sprinterplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/experimental/sprinter)
install(FILES qml/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/experimental/sprinter)

set(SPRINTER_PLUGINS_PATH plugins/sprinter)

### Runners
add_subdirectory(runners)
