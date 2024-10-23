cmake_minimum_required(VERSION 3.5.0)
include(${CMAKE_SOURCE_DIR}/MakeScripts/cmake/CommonSettings.cmake)
include(${CMAKE_SOURCE_DIR}/MakeScripts/cmake/AddExternalPkg.cmake)
include(${CMAKE_SOURCE_DIR}/MakeScripts/cmake/PrePostBuildCommands.cmake)
apply_settings_before_project()
project(Engine)

# Add header files
file(GLOB_RECURSE HEADER ${CMAKE_SOURCE_DIR}/src/lve/*.hpp)

# Add source files
file(GLOB_RECURSE SOURCE ${CMAKE_SOURCE_DIR}/src/lve/*.cpp)

# Add library
add_library(${PROJECT_NAME} STATIC ${SOURCE} ${HEADER})

# Apply common settings
apply_common_settings(${PROJECT_NAME})

# Add include directories
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src)

# Add external libraries
add_external_libs_lve(${PROJECT_NAME})

# Pre build command
set_pre_build_command(${PROJECT_NAME})
