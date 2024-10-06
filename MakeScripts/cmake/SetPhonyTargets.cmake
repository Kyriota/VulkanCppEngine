function(set_phony_targets target_name)
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

    add_custom_target(setup_runtime_environment
        COMMAND ${CMAKE_COMMAND} -E echo "Running make scripts for WIN32..."
        COMMAND ${CMAKE_SOURCE_DIR}/MakeScripts/win32/setup_runtime_environment/run.bat ${IS_DEBUG} ${RUNTIME_OUTPUT_DIRECTORY} ${CMAKE_SOURCE_DIR}/shaders ${CMAKE_SOURCE_DIR}/assets ${CMAKE_SOURCE_DIR}/config ${DYNAMIC_LIB_DIR}
        VERBATIM
    )

    add_custom_target(clean_bin
        COMMAND ${CMAKE_COMMAND} -E echo "Cleaning bin directory..."
        # remove files in BINARY_DIR
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${RUNTIME_OUTPUT_DIRECTORY}
        # recreate BINARY_DIR
        COMMAND ${CMAKE_COMMAND} -E make_directory ${RUNTIME_OUTPUT_DIRECTORY}
        VERBATIM
    )

    add_custom_target(backup_runtime_files
        COMMAND ${CMAKE_COMMAND} -E echo "Copying runtime files to backup directory..."
        COMMAND ${CMAKE_COMMAND} -E make_directory ${RUNTIME_OUTPUT_DIRECTORY} # in case backup directory is not created
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${RUNTIME_OUTPUT_DIRECTORY} ${BACKUP_DIR}
        VERBATIM
    )

    add_custom_target(restore_runtime_files
        COMMAND ${CMAKE_COMMAND} -E echo "Restoring runtime files from backup directory..."
        COMMAND ${CMAKE_COMMAND} -E make_directory ${BACKUP_DIR} # in case backup directory is not created
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${BACKUP_DIR} ${RUNTIME_OUTPUT_DIRECTORY}
        VERBATIM
    )

endfunction()
