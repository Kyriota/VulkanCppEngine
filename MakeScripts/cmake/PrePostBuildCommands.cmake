function(set_pre_build_command target_name)
    add_custom_command( # pre build commands
        TARGET ${target_name}
        PRE_BUILD
        
        COMMAND ${CMAKE_COMMAND} -E echo "Cleaning bin directory..."
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${RUNTIME_OUTPUT_DIRECTORY}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${RUNTIME_OUTPUT_DIRECTORY}

        VERBATIM
    )
endfunction()

function(set_post_build_command target_name)
    if (WIN32)
        set (SCRIPT_PATH ${CMAKE_SOURCE_DIR}/MakeScripts/win32/setup_runtime_environment/run.bat)
    else()
        message(FATAL_ERROR "Unsupported OS")
    endif()

    add_custom_command( # post build commands
        TARGET ${target_name}
        POST_BUILD

        COMMAND ${CMAKE_COMMAND} -E echo "Setting up runtime environment in ${RUNTIME_OUTPUT_DIRECTORY}..."
        COMMAND ${SCRIPT_PATH} ${RUNTIME_OUTPUT_DIRECTORY} ${CMAKE_SOURCE_DIR}/shaders ${CMAKE_SOURCE_DIR}/assets ${CMAKE_SOURCE_DIR}/config

        VERBATIM
    )
endfunction()
