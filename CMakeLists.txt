cmake_minimum_required(VERSION 2.8)

find_package(ECM 0.0.8 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)

find_package(Qt5Core REQUIRED)
find_package(Qt5Declarative REQUIRED)

set(CMAKE_AUTOMOC ON)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/lib ${CMAKE_BINARY_DIR})

set(sprinterlib_SRCS
    lib/abstractrunner.cpp
    lib/querymatch.cpp
    lib/runnercontext.cpp
    lib/runnermanager.cpp
    lib/runnermanagerthread.cpp
    lib/runnersessiondata.cpp
)

set(runners_SRCS
    runners/datetime/datetime.cpp
    runners/c/c.cpp
    runners/youtube/youtube.cpp
)

set(sprinter_SRCS
    ${sprinterlib_SRCS}
    ${runners_SRCS}
    test/main.cpp
    test/simulator.cpp
)

set(sprinterplugin_SRCS
    ${sprinterlib_SRCS}
    ${runners_SRCS}
    qml/sprinterplugin.cpp
)

add_executable(sprinter ${sprinter_SRCS})
qt5_use_modules(sprinter Widgets Network)

add_library(sprinterplugin SHARED ${sprinterplugin_SRCS})
qt5_use_modules(sprinterplugin Widgets Network Declarative)

install(TARGETS sprinter DESTINATION bin)

install(TARGETS sprinterplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/experimental/sprinter)
install(FILES qml/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/experimental/sprinter)
