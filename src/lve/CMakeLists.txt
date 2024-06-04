cmake_minimum_required(VERSION 3.5.0)
project(Engine)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Add source files
file(GLOB_RECURSE SOURCE ${CMAKE_SOURCE_DIR}/src/lve/*.cpp)

# Add library
add_library(${PROJECT_NAME} SHARED ${SOURCE})

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/build/Debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/build/Release
)

# Get the current configuration
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CONFIGURATION "DEBUG")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CONFIGURATION "RELEASE")
else()
    message(FATAL_ERROR "Unsupported configuration")
endif()

# Set RUNTIME_OUTPUT_DIRECTORY
set(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/${CONFIGURATION})

# Add include directories
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src)

# Add external libraries
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/external/include)

# ==================== Link libraries ====================
# Set library directories
set(LIBRARY_DIR ${CMAKE_SOURCE_DIR}/external/lib)
# Link vulkan lib
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBRARY_DIR}/vulkan-1.lib)
# Link yaml-cpp lib
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBRARY_DIR}/libyaml-cpp.dll.a)
# Link glfw lib
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBRARY_DIR}/libglfw3.a)

# Link OpenMP
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX)
else()
    message(FATAL_ERROR "OpenMP not found")
endif()