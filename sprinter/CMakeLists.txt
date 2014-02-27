project(sprinter)

set(sprinterlib_SRCS
    matchdata.cpp
    querymatch.cpp
    querycontext.cpp
    querysession.cpp
    querysessionthread_p.cpp
    runner.cpp
    runnermodel_p.cpp
    runnersessiondata.cpp
)

add_library(sprinter SHARED ${sprinterlib_SRCS})
set_target_properties(sprinter PROPERTIES VERSION ${SPRINTER_VERSION_STRING}
                                               SOVERSION ${SPRINTER_SOVERSION}
                                               EXPORT_NAME Sprinter
)

ecm_generate_headers(sprinterlib_HEADERS
    HEADER_NAMES
        MatchData
        QueryContext
        QueryMatch
        QuerySession
        Runner
        RunnerSessionData
    REQUIRED_HEADERS sprinterlib_HEADERS
)
generate_export_header(sprinter BASE_NAME Sprinter EXPORT_FILE_NAME sprinter_export.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

qt5_use_modules(sprinter Widgets Network)
install(TARGETS sprinter EXPORT SprinterTargets DESTINATION lib)
install(FILES
            ${sprinterlib_HEADERS}
            ${CMAKE_CURRENT_BINARY_DIR}/sprinter_export.h
        DESTINATION include/sprinter/)
