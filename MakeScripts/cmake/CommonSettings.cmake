function(set_output_directories target_name)
    # Set output directory
    set_target_properties(${target_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/build/Debug
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/build/Release
    )

    # Set RUNTIME_OUTPUT_DIRECTORY
    set(RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE} PARENT_SCOPE)
endfunction()



function(set_cpp_standard target_name)
    # Set C++ standard
    target_compile_features(${target_name} PUBLIC cxx_std_20)
endfunction()
