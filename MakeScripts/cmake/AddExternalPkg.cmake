function(add_external_libs_lve target_name)
    if (MSVC)
        # Add external include directories
        target_include_directories(${target_name} PUBLIC ${CMAKE_SOURCE_DIR}/build/vcpkg_installed/x64-windows-static/include)
        # Add external link directories
        link_directories(${CMAKE_SOURCE_DIR}/build/vcpkg_installed/x64-windows-static/lib)
        # Set VCPKG target triplet
        set(VCPKG_TARGET_TRIPLET x64-windows-static)
        set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/build/vcpkg_installed/x64-windows-static/share)
    elseif (MINGW)
        # Add external include directories
        target_include_directories(${target_name} PUBLIC ${CMAKE_SOURCE_DIR}/build/vcpkg_installed/x64-mingw-static/include)
        # Add external link directories
        link_directories(${CMAKE_SOURCE_DIR}/build/vcpkg_installed/x64-mingw-static/lib)
        # Set VCPKG target triplet
        set(VCPKG_TARGET_TRIPLET x64-mingw-static)
        set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/build/vcpkg_installed/x64-mingw-static/share)
    else()
        message(FATAL_ERROR "Unsupported compiler")
    endif()

    # Install vcpkg packages
    execute_process(
        COMMAND vcpkg install --triplet ${VCPKG_TARGET_TRIPLET} --x-install-root ${CMAKE_SOURCE_DIR}/build/vcpkg_installed
    )

    # ==================== Link Static libraries ====================
    # Link glm lib
    find_package(glm CONFIG REQUIRED)
    target_link_libraries(${target_name} PUBLIC glm::glm)
    # Link glfw lib
    find_package(glfw3 CONFIG REQUIRED)
    target_link_libraries(${target_name} PUBLIC glfw)
    # Link yaml-cpp lib
    find_package(yaml-cpp CONFIG REQUIRED)
    target_link_libraries(${target_name} PUBLIC yaml-cpp::yaml-cpp)

    # Link OMP
    find_package(OpenMP REQUIRED)
    target_link_libraries(${target_name} PUBLIC OpenMP::OpenMP_CXX)

    # Set VulkanSDK directory
    set(VULKAN_SDK $ENV{VULKAN_SDK})
    # Replace backslashes with forward slashes
    string(REPLACE "\\" "/" VULKAN_SDK ${VULKAN_SDK})
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
    
endfunction()
