cmake_minimum_required(VERSION 3.5.0)
project(vulkan-cpp_engine)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)

# Add source files cpp and header files hpp
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.hpp")

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Add include directories
target_include_directories(${PROJECT_NAME} PUBLIC src)

# Add external libraries
target_include_directories(${PROJECT_NAME} PUBLIC external/include)

# Link vulkan lib
target_link_libraries(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/external/lib/vulkan-1.lib)
# Link glfw lib
if(CMAKE_GENERATOR MATCHES "Visual Studio 17 2022")
    set(GLFW_LIB ${CMAKE_SOURCE_DIR}/external/lib/glfw3.lib)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(GLFW_LIB ${CMAKE_SOURCE_DIR}/external/lib/libglfw3.a)
else()
    message(FATAL_ERROR "Unsupported compiler")
endif()
target_link_libraries(${PROJECT_NAME} PUBLIC ${GLFW_LIB})

# Compile shaders
add_custom_target(rebuild_shaders
    COMMAND ${CMAKE_COMMAND} -E echo "Rebuilding shaders..."
    COMMAND ${CMAKE_SOURCE_DIR}/compile_shaders.bat
    RESULT_VARIABLE SHADER_COMPILE_RESULT
    VERBATIM
)

add_dependencies(${PROJECT_NAME} rebuild_shaders)

# Change working directory in MSVC
if(CMAKE_GENERATOR MATCHES "Visual Studio 17 2022")
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif()