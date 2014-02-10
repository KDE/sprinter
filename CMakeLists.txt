cmake_minimum_required(VERSION 2.8)
include (GenerateExportHeader)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)

find_package(Qt5Core REQUIRED)
find_package(Qt5Declarative REQUIRED)

set(CMAKE_AUTOMOC ON)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

add_definitions("-Wall -std=c++0x")

### Main library
set(sprinterlib_SRCS
    sprinter/abstractrunner.cpp
    sprinter/querymatch.cpp
    sprinter/querycontext.cpp
    sprinter/querysession.cpp
    sprinter/querysessionthread_p.cpp
    sprinter/runnermodel_p.cpp
    sprinter/runnersessiondata.cpp
)

set(sprinterlib_HEADERS
    sprinter/abstractrunner.h
    sprinter/querycontext.h
    sprinter/querymatch.h
    sprinter/querysession.h
    sprinter/runnersessiondata.h
    ${CMAKE_BINARY_DIR}/sprinter/sprinter_export.h
)

add_library(sprinter SHARED ${sprinterlib_SRCS})
generate_export_header(sprinter BASE_NAME Sprinter EXPORT_FILE_NAME sprinter/sprinter_export.h)
qt5_use_modules(sprinter Widgets Network)
install(TARGETS sprinter DESTINATION lib)

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
