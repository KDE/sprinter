cmake_minimum_required(VERSION 2.8)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)

find_package(Qt5Core REQUIRED)
find_package(Qt5Declarative REQUIRED)

set(CMAKE_AUTOMOC ON)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/lib ${CMAKE_BINARY_DIR})

### Main library
set(sprinterlib_SRCS
    lib/abstractrunner.cpp
    lib/querymatch.cpp
    lib/querycontext.cpp
    lib/runnermanager.cpp
    lib/runnermanagerthread_p.cpp
    lib/runnermodel_p.cpp
    lib/runnersessiondata.cpp
)

add_library(sprinter SHARED ${sprinterlib_SRCS})
qt5_use_modules(sprinter Widgets Network)
install(TARGETS sprinter DESTINATION lib)

### Test app - temporary, will be removed at some point
set(sprinter_SRCS
    test/main.cpp
)

add_executable(sprintertest ${sprinter_SRCS})
qt5_use_modules(sprintertest Widgets Network)
target_link_libraries(sprintertest sprinter)


### QML plugins
set(qmlsprinterplugin_SRCS
    qml/sprinterplugin.cpp
)

add_library(sprinterplugin SHARED ${qmlsprinterplugin_SRCS})
qt5_use_modules(sprinterplugin Widgets Network Declarative)
target_link_libraries(sprinterplugin sprinter)

install(TARGETS sprintertest DESTINATION bin)

install(TARGETS sprinterplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/experimental/sprinter)
install(FILES qml/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/experimental/sprinter)

set(SPRINTER_PLUGINS_PATH plugins/sprinter)
add_subdirectory(runners)
