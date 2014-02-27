cmake_minimum_required(VERSION 2.8)

# ECM setup
find_package(ECM 0.0.10 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH
        ${ECM_MODULE_PATH}
        ${ECM_KDE_MODULE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
        ${CMAKE_MODULE_PATH})

include(FeatureSummary)
include(GenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMPackageConfigHelpers)
# include(KDEInstallDirs)
# include(KDEFrameworkCompilerSettings)
# include(KDECMakeSettings)

ecm_setup_version(0.2
    VARIABLE_PREFIX SPRINTER
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/sprinter_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/SprinterConfigVersion.cmake"
)

find_package(Qt5Core REQUIRED)
find_package(Qt5Declarative REQUIRED)

set(CMAKE_AUTOMOC ON)
add_definitions("-Wall -std=c++0x")
set(SPRINTER_PLUGINS_PATH plugins/sprinter)
include_directories(${CMAKE_SOURCE_DIR})
# the main library
add_subdirectory(sprinter)

# qml plugin
add_subdirectory(qml)

# test apps
add_subdirectory(test)

# Create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_DIR}/Sprinter")
set(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_DIR}/include/Sprinter")

ecm_configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/SprinterConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/SprinterConfig.cmake"
    INSTALL_DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
)

install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/SprinterConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/SprinterConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel)

install(EXPORT SprinterTargets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE SprinterTargets.cmake)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/sprinter_version.h"
        DESTINATION "${INCLUDE_INSTALL_DIR}")

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
