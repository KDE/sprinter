project(sprinter_qmlplugin)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

### QML plugins
set(qmlsprinterplugin_SRCS
    sprinterplugin.cpp
)

add_library(sprinterplugin SHARED ${qmlsprinterplugin_SRCS})
qt5_use_modules(sprinterplugin Widgets Network Declarative)
target_link_libraries(sprinterplugin sprinter)

install(TARGETS sprinterplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/experimental/sprinter)
install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/experimental/sprinter)

