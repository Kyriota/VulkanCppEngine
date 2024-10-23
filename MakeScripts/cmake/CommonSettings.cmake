function(configure_MSVC target_name type)
    if (NOT MSVC)
        return()
    endif()

    # Use generator expressions to set the correct runtime library option for different configurations
    if (type STREQUAL "dynamic")
        target_compile_options(${target_name} PRIVATE
            "$<$<CONFIG:Debug>:/MDd>"  # Debug mode with dynamic runtime
            "$<$<CONFIG:Release>:/MD>" # Release mode with dynamic runtime
        )
    elseif (type STREQUAL "static")
        target_compile_options(${target_name} PRIVATE
            "$<$<CONFIG:Debug>:/MTd>"  # Debug mode with static runtime
            "$<$<CONFIG:Release>:/MT>" # Release mode with static runtime
        )
    else()
        message(FATAL_ERROR "Invalid MSVC runtime library type")
    endif()

    # Set VS debugger working directory
    set_target_properties(${target_name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin)
endfunction()


function(apply_common_settings target_name)
    # Set runtime output directory
    set(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin/ PARENT_SCOPE)

    # Set C++ standard
    target_compile_features(${target_name} PUBLIC cxx_std_20)

    # Set MSVC runtime library type
    configure_MSVC(${target_name} "static")
endfunction()

function(apply_settings_before_project)
    # Set VCPKG manifest install to off
    set(VCPKG_MANIFEST_INSTALL OFF CACHE BOOL "VCPKG manifest install" FORCE)

    # Set VCPKG toolchain
    set(VCPKG_ROOT $ENV{VCPKG_ROOT})
    # Replace backslashes with forward slashes
    string(REPLACE "\\" "/" VCPKG_ROOT ${VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE ${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake PARENT_SCOPE)
endfunction()
