cmake_minimum_required(VERSION 3.5.0)
project(vulkan-cpp-engine)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Add source files cpp and header files hpp
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.hpp")
file(GLOB_RECURSE TEMPLATES "src/*.tpp")

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

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
    # Handle other build types if necessary
endif()

# Set RUNTIME_OUTPUT_DIRECTORY
set(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/${CONFIGURATION})

# Add include directories
target_include_directories(${PROJECT_NAME} PUBLIC src)

# Add external libraries
target_include_directories(${PROJECT_NAME} PUBLIC external/include)

# ==================== Link libraries ====================
# Set library directories
set(LIBRARY_DIR ${CMAKE_SOURCE_DIR}/external/lib)
# Link vulkan lib
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBRARY_DIR}/vulkan-1.lib)
# Link yaml-cpp lib
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBRARY_DIR}/libyaml-cpp.a)
# Link glfw lib
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBRARY_DIR}/libglfw3.a)

# Link OpenMP
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX)
else()
    message(FATAL_ERROR "OpenMP not found")
endif()

# Other scripts
if(WIN32)
    set(SCRIPT_SUFFIX .bat)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

add_custom_target(make_scripts
    COMMAND ${CMAKE_COMMAND} -E echo "Running make scripts..."
    COMMAND ${CMAKE_SOURCE_DIR}/run_make_scripts${SCRIPT_SUFFIX} ${RUNTIME_OUTPUT_DIRECTORY} ${CMAKE_SOURCE_DIR}/assets/shaders ${CMAKE_SOURCE_DIR}/assets ${CMAKE_SOURCE_DIR}/config
    RESULT_VARIABLE MAKE_SCRIPTS_RESULT
    VERBATIM
)

add_dependencies(${PROJECT_NAME} make_scripts)