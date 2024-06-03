cmake_minimum_required(VERSION 3.5.0)
project(vulkan-cpp_engine)

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

# Set library directories
set(LIBRARY_DIR ${CMAKE_SOURCE_DIR}/external/lib)
# Link vulkan lib
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBRARY_DIR}/vulkan-1.lib)
# Link yaml-cpp lib
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBRARY_DIR}/libyaml-cpp.a)
# Link glfw lib
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBRARY_DIR}/libglfw3.a)

# Other scripts
if(WIN32)
    set(SCRIPTS_DIR ${CMAKE_SOURCE_DIR}/MakeScripts/win32)
    set(SCRIPT_SUFFIX .bat)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

# Compile shaders
add_custom_target(compile_shaders
    COMMAND ${CMAKE_COMMAND} -E echo "Compiling shaders..."
    COMMAND ${SCRIPTS_DIR}/compile_shaders${SCRIPT_SUFFIX} ${RUNTIME_OUTPUT_DIRECTORY} ${CMAKE_SOURCE_DIR}/assets/shaders
    RESULT_VARIABLE SHADER_COMPILE_RESULT
    VERBATIM
)

add_dependencies(${PROJECT_NAME} compile_shaders)

# Copy assets
add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E echo "Copying assets..."
    COMMAND ${SCRIPTS_DIR}/copy_assets${SCRIPT_SUFFIX} ${RUNTIME_OUTPUT_DIRECTORY} ${CMAKE_SOURCE_DIR}/assets
    RESULT_VARIABLE ASSETS_COPY_RESULT
    VERBATIM
)

add_dependencies(${PROJECT_NAME} copy_assets)

# Copy dlls
add_custom_target(copy_dlls
    COMMAND ${CMAKE_COMMAND} -E echo "Copying dlls..."
    COMMAND ${SCRIPTS_DIR}/copy_dlls${SCRIPT_SUFFIX} ${RUNTIME_OUTPUT_DIRECTORY}
    RESULT_VARIABLE DLLS_COPY_RESULT
    VERBATIM
)

add_dependencies(${PROJECT_NAME} copy_dlls)

# Copy config
add_custom_target(copy_config
    COMMAND ${CMAKE_COMMAND} -E echo "Copying config..."
    COMMAND ${SCRIPTS_DIR}/copy_config${SCRIPT_SUFFIX} ${RUNTIME_OUTPUT_DIRECTORY} ${CMAKE_SOURCE_DIR}/config
    RESULT_VARIABLE CONFIGS_COPY_RESULT
    VERBATIM
)

add_dependencies(${PROJECT_NAME} copy_config)