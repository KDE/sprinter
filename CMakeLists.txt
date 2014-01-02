cmake_minimum_required(VERSION 2.8)
find_package(Qt5Core REQUIRED)
set(CMAKE_AUTOMOC ON)
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/lib ${CMAKE_BINARY_DIR})

#FOR RUNNERS
find_package(QJSON REQUIRED)
#END FOR RUNNERS

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

add_executable(sprinter ${sprinter_SRCS})
qt5_use_modules(sprinter Widgets Network)

#FOR RUNNERS
target_link_libraries(sprinter ${QJSON_LIBRARIES})
#END FOR RUNNERS

install(TARGETS sprinter DESTINATION bin)
