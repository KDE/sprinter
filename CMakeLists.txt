cmake_minimum_required(VERSION 2.8)
find_package(KDE4 REQUIRED)

include(KDE4Defaults)
include_directories( ${KDE4_INCLUDES} ${QT_INCLUDES} ${CMAKE_SOURCE_DIR}/lib)

set(runnermanager_SRCS
    abstractrunner.cpp
    main.cpp
    runnerhelper.cpp
    runnermanager.cpp
    runnermanagerthread.cpp
    runners.cpp
    simulator.cpp
)

KDE4_ADD_EXECUTABLE(runnermanager ${runnermanager_SRCS})

target_link_libraries(runnermanager ${QT_QTCORE_LIBRARY})

install(TARGETS runnermanager ${INSTALL_TARGETS_DEFAULT_ARGS} )


