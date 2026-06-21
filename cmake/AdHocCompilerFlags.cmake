# Applies the project-standard compile options to a target.
# Reads the active configuration from CMAKE_BUILD_TYPE (single-config trees).
function(adhoc_apply_compiler_flags target)
    target_compile_features(${target} PRIVATE cxx_std_23)

    if(APPLE)
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -fexceptions
            -frtti
        )
    elseif(WIN32)
        target_compile_options(${target} PRIVATE
            /utf-8
            /W4
            /EHsc   # Exceptions
            /GR     # RTTI on
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
            # Run the ThinLTO backend codegen at -O3 to match the per-module level.
            target_link_options(${target} PRIVATE -O3)
        endif()
    elseif(WIN32)
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            target_compile_options(${target} PRIVATE /Od /Zi)
            target_link_options(${target} PRIVATE /DEBUG)
        elseif(CMAKE_BUILD_TYPE STREQUAL "Dev")
            target_compile_options(${target} PRIVATE /O2 /Zi)
            target_link_options(${target} PRIVATE /DEBUG)
        else()
            target_compile_options(${target} PRIVATE /clang:-O3)
            target_compile_definitions(${target} PRIVATE NDEBUG)
            # Run the lld-link ThinLTO backend codegen at -O3 (default is /opt:lldlto=2).
            target_link_options(${target} PRIVATE /opt:lldlto=3)
        endif()
    endif()

    # ThinLTO for Release across both toolchains. CMake derives the matching
    # compile, archive, and link flags from this property, which is why the
    # static-library archive step stays correct. The top-level CMakeLists
    # validates IPO support for Release, so this is always safe to set here.
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set_property(TARGET ${target} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endfunction()
