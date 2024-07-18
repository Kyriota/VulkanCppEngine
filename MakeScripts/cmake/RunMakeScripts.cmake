function(run_make_scripts target_name)
    if(!WIN32)
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
        COMMAND ${CMAKE_COMMAND} -E echo "Running make scripts for WIN32..."
        COMMAND ${CMAKE_SOURCE_DIR}/MakeScripts/win32/run_make_scripts.bat ${IS_DEBUG} ${RUNTIME_OUTPUT_DIRECTORY} ${CMAKE_SOURCE_DIR}/assets/shaders ${CMAKE_SOURCE_DIR}/assets ${CMAKE_SOURCE_DIR}/config ${DYNAMIC_LIB_DIR}
        VERBATIM
    )

    add_dependencies(${target_name} make_scripts)
endfunction()