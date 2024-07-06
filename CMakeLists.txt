cmake_minimum_required(VERSION 3.5.0)
project(vulkan-cpp-engine)
include(${CMAKE_SOURCE_DIR}/MakeScripts/cmake/CommonSettings.cmake)

# Engine
add_subdirectory(${CMAKE_SOURCE_DIR}/src/lve)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Add source files
file(GLOB_RECURSE APP_SRC ${CMAKE_SOURCE_DIR}/src/app/*.cpp)
file(GLOB_RECURSE APP_HDR ${CMAKE_SOURCE_DIR}/src/app/*.hpp)
set(SOURCE ${APP_SRC} ${CMAKE_SOURCE_DIR}/src/main.cpp)
set(HEADER ${APP_HDR})

# Add executable
add_executable(${PROJECT_NAME} ${SOURCE} ${HEADER})

# Set output deirectories
set_output_directories(${PROJECT_NAME})

# Add include directories
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src)

# Link Engine
target_link_libraries(${PROJECT_NAME} Engine)

# ==================== Other scripts ====================
if(WIN32)
    set(SCRIPT_SUFFIX .bat)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(IS_DEBUG 1)
    set(DYNAMIC_LIB_DIR ${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-mingw-dynamic/debug/bin)
else()
    set(IS_DEBUG 0)
    set(DYNAMIC_LIB_DIR ${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-mingw-dynamic/bin)
endif()

add_custom_target(make_scripts
    COMMAND ${CMAKE_COMMAND} -E echo "Running make scripts..."
    COMMAND ${CMAKE_SOURCE_DIR}/run_make_scripts${SCRIPT_SUFFIX} ${IS_DEBUG} ${RUNTIME_OUTPUT_DIRECTORY} ${CMAKE_SOURCE_DIR}/assets/shaders ${CMAKE_SOURCE_DIR}/assets ${CMAKE_SOURCE_DIR}/config ${DYNAMIC_LIB_DIR}
    RESULT_VARIABLE MAKE_SCRIPTS_RESULT
    VERBATIM
)

add_dependencies(${PROJECT_NAME} make_scripts)