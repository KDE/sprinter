cmake_minimum_required(VERSION 2.8)
find_package(Qt5Core)
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
    runners/a/a.cpp
    runners/b/b.cpp
    runners/c/c.cpp
    runners/d/d.cpp
)

set(sprinter_SRCS
    ${sprinterlib_SRCS}
    ${runners_SRCS}
    test/main.cpp
    test/simulator.cpp
)

add_executable(sprinter ${sprinter_SRCS})
qt5_use_modules(sprinter Core)
install(TARGETS sprinter DESTINATION bin)
