cmake_minimum_required(VERSION 3.5.0)
project(Engine)
include(${CMAKE_SOURCE_DIR}/MakeScripts/cmake/CommonSettings.cmake)
include(${CMAKE_SOURCE_DIR}/MakeScripts/cmake/AddExternalPkg.cmake)

# Add source files
file(GLOB_RECURSE SOURCE ${CMAKE_SOURCE_DIR}/src/lve/*.cpp)

# Add library
add_library(${PROJECT_NAME} SHARED ${SOURCE})

# Set C++ standard
set_cpp_standard(${PROJECT_NAME})

# Set output deirectories
set_output_directories(${PROJECT_NAME})

# Add include directories
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src)

# Add external libraries
add_external_libs_lve(${PROJECT_NAME})