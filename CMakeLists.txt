cmake_minimum_required(VERSION 3.5.0)
project(vulkan-cpp-engine)
include(${CMAKE_SOURCE_DIR}/MakeScripts/cmake/CommonSettings.cmake)
include(${CMAKE_SOURCE_DIR}/MakeScripts/cmake/SetPhonyTargets.cmake)

# Engine
add_subdirectory(${CMAKE_SOURCE_DIR}/src/lve)

# Add source files
file(GLOB_RECURSE APP_SRC ${CMAKE_SOURCE_DIR}/src/app/*.cpp)
file(GLOB_RECURSE APP_HDR ${CMAKE_SOURCE_DIR}/src/app/*.hpp)
set(SOURCE ${APP_SRC} ${CMAKE_SOURCE_DIR}/src/main.cpp)
set(HEADER ${APP_HDR})

# Add executable
add_executable(${PROJECT_NAME} ${SOURCE} ${HEADER})

# Set C++ standard
set_cpp_standard(${PROJECT_NAME})

# Set output deirectories
set_output_directories(${PROJECT_NAME})

# Add include directories
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src)

# Link Engine
target_link_libraries(${PROJECT_NAME} Engine)

# Run make scripts
set_phony_targets(${PROJECT_NAME})
