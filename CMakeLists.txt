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

add_definitions("-Wall -std=c++0x")
set(SPRINTER_PLUGINS_PATH plugins/sprinter)

# the main library
add_subdirectory(sprinter)

# qml plugin
add_subdirectory(qml)

# test apps
add_subdirectory(test)

### Runners
add_subdirectory(runners)
