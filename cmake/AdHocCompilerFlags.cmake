# Applies the project-standard compile options to a target.
# Reads the active configuration from CMAKE_BUILD_TYPE (single-config trees).
function(adhoc_apply_compiler_flags target)
    target_compile_features(${target} PRIVATE cxx_std_23)

    if(APPLE)
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -Wno-unused-parameter
            -fexceptions
            -frtti
        )
    elseif(WIN32)
        target_compile_options(${target} PRIVATE
            /utf-8
            /W4
            /wd4100       # unreferenced formal parameter (matches -Wno-unused-parameter)
            /permissive-
            /EHsc
            /GR           # RTTI on
            /Zc:__cplusplus
            /Zc:preprocessor
        )
    endif()
endfunction()

# Applies optimisation flags that match the active build type's intent.
# Debug: no optimisation, debug info.
# Dev:   optimised + debug info.
# Release: optimised, no debug info, NDEBUG.
function(adhoc_apply_variant_flags target)
    if(APPLE)
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            target_compile_options(${target} PRIVATE -O0 -g)
        elseif(CMAKE_BUILD_TYPE STREQUAL "Dev")
            target_compile_options(${target} PRIVATE -O2 -g)
        else()
            target_compile_options(${target} PRIVATE -O3)
            target_compile_definitions(${target} PRIVATE NDEBUG)
        endif()
    elseif(WIN32)
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            target_compile_options(${target} PRIVATE /Od /Zi)
            target_link_options(${target} PRIVATE /DEBUG)
        elseif(CMAKE_BUILD_TYPE STREQUAL "Dev")
            target_compile_options(${target} PRIVATE /O2 /Zi)
            target_link_options(${target} PRIVATE /DEBUG)
        else()
            target_compile_options(${target} PRIVATE /O3)
            target_compile_definitions(${target} PRIVATE NDEBUG)
        endif()
    endif()
endfunction()
