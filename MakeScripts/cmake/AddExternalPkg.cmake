function(add_external_libs_lve target_name)
    # Add external include directories
    target_include_directories(${target_name} PUBLIC ${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-mingw-static/include)
    target_include_directories(${target_name} PUBLIC ${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-mingw-dynamic/include)

    # Add external link directories
    link_directories(${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-mingw-static/lib)
    link_directories(${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-mingw-dynamic/lib)

    # ==================== Link Static libraries ====================
    set(VCPKG_TARGET_TRIPLET x64-mingw-static)
    set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-mingw-static/share)
    # Link glm lib
    find_package(glm CONFIG REQUIRED)
    target_link_libraries(${target_name} PUBLIC glm::glm-header-only)
    # Link glfw lib
    find_package(glfw3 CONFIG REQUIRED)
    target_link_libraries(${target_name} PUBLIC glfw)

    # Set VulkanSDK directory
    set(VULKAN_SDK $ENV{VULKAN_SDK})
    # Check if VulkanSDK directory is set
    if(EXISTS ${VULKAN_SDK})
        # Add Vulkan include directories
        target_include_directories(${target_name} PUBLIC ${VULKAN_SDK}/Include)
        # Add Vulkan link directories
        link_directories(${VULKAN_SDK}/Lib)
        # Link Vulkan lib
        target_link_libraries(${target_name} PUBLIC ${VULKAN_SDK}/Lib/vulkan-1.lib)
    else()
        message(FATAL_ERROR "VulkanSDK directory is not set. Make sure VulkanSDK is installed and VULKAN_SDK environment variable is set.")
    endif()

    # ==================== Link Dynamic libraries ====================
    set(VCPKG_TARGET_TRIPLET x64-mingw-dynamic)
    set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-mingw-dynamic/share)
    # Link yaml-cpp lib
    find_package(yaml-cpp CONFIG REQUIRED)
    target_link_libraries(${target_name} PUBLIC yaml-cpp::yaml-cpp)
endfunction()