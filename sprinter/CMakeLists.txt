project(sprinter)

set(sprinterlib_SRCS
    querymatch.cpp
    querycontext.cpp
    querysession.cpp
    querysessionthread_p.cpp
    runner.cpp
    runnermodel_p.cpp
    runnersessiondata.cpp
)

set(sprinterlib_HEADERS
    querycontext.h
    querymatch.h
    querysession.h
    runner.h
    runnersessiondata.h
    ${CMAKE_BINARY_DIR}/sprinter/sprinter_export.h
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_library(sprinter SHARED ${sprinterlib_SRCS})
generate_export_header(sprinter BASE_NAME Sprinter EXPORT_FILE_NAME sprinter_export.h)
qt5_use_modules(sprinter Widgets Network)
install(TARGETS sprinter DESTINATION lib)
install(FILES ${sprinterlib_HEADERS} DESTINATION include/sprinter/)
