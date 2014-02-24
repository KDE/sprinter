project(sprinter_testapps)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

### Test app - temporary, will be removed at some point
set(sprinter_SRCS
    helper.cpp
    main.cpp
)
add_executable(sprintertest ${sprinter_SRCS})
qt5_use_modules(sprintertest Widgets Network)
target_link_libraries(sprintertest sprinter)
install(TARGETS sprintertest DESTINATION bin)
