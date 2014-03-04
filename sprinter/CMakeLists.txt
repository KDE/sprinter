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

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_library(sprinter SHARED ${sprinterlib_SRCS})
target_include_directories(sprinter INTERFACE "${INCLUDE_INSTALL_DIR}")
set_target_properties(sprinter
                        PROPERTIES VERSION ${SPRINTER_VERSION_STRING}
                        SOVERSION ${SPRINTER_SOVERSION}
                        EXPORT_NAME "Sprinter"
)

ecm_generate_headers(sprinterlib_HEADERS
    HEADER_NAMES
        MatchData
        QueryContext
        QueryMatch
        QuerySession
        Runner
        RunnerSessionData
    PREFIX Sprinter
    REQUIRED_HEADERS sprinterlib_SMALL_HEADERS
)
generate_export_header(sprinter BASE_NAME Sprinter EXPORT_FILE_NAME sprinter_export.h)

qt5_use_modules(sprinter Gui Network)
install(TARGETS sprinter EXPORT SprinterTargets DESTINATION lib)
install(FILES
            ${sprinterlib_HEADERS}
        DESTINATION include/sprinter/Sprinter/)
install(FILES
            ${sprinterlib_SMALL_HEADERS}
            ${CMAKE_CURRENT_BINARY_DIR}/sprinter_export.h
        DESTINATION include/sprinter/sprinter/)
