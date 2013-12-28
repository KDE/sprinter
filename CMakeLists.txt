cmake_minimum_required(VERSION 2.8)
find_package(KDE4 REQUIRED)

include(KDE4Defaults)
include_directories( ${KDE4_INCLUDES} ${QT_INCLUDES} ${CMAKE_SOURCE_DIR}/lib)

set(runnermanagerlib_SRCS
    lib/abstractrunner.cpp
    lib/querymatch.cpp
    lib/runnercontext.cpp
    lib/runnerhelper.cpp
    lib/runnermanager.cpp
    lib/runnermanagerthread.cpp
)

set(runners_SRCS
    runners/a/a.cpp
    runners/b/b.cpp
    runners/c/c.cpp
    runners/d/d.cpp
)

set(runnermanager_SRCS
    ${runnermanagerlib_SRCS}
    ${runners_SRCS}
    test/main.cpp
    test/simulator.cpp
)

KDE4_ADD_EXECUTABLE(runnermanager ${runnermanager_SRCS})

target_link_libraries(runnermanager ${QT_QTCORE_LIBRARY})

install(TARGETS runnermanager ${INSTALL_TARGETS_DEFAULT_ARGS} )
