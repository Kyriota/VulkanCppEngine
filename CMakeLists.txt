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
# Link yaml-cpp lib
target_link_libraries(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/external/lib/libyaml-cpp.a)
# Link glfw lib
target_link_libraries(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/external/lib/libglfw3.a)

# Compile shaders
add_custom_target(rebuild_shaders
    COMMAND ${CMAKE_COMMAND} -E echo "Rebuilding shaders..."
    COMMAND ${CMAKE_SOURCE_DIR}/compile_shaders.bat
    RESULT_VARIABLE SHADER_COMPILE_RESULT
    VERBATIM
)

add_dependencies(${PROJECT_NAME} rebuild_shaders)