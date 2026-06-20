# Applies the standard per-config and per-platform compile definitions to a target.
# Reads the active configuration from CMAKE_BUILD_TYPE (single-config trees).
function(adhoc_apply_config_defines target)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_definitions(${target} PRIVATE ADHOC_DEBUG=1 ADHOC_DEV=0 ADHOC_RELEASE=0)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Dev")
        target_compile_definitions(${target} PRIVATE ADHOC_DEBUG=0 ADHOC_DEV=1 ADHOC_RELEASE=0)
    else()
        target_compile_definitions(${target} PRIVATE ADHOC_DEBUG=0 ADHOC_DEV=0 ADHOC_RELEASE=1)
    endif()
    # All first-party targets get internal API access (e.g. MutableInstance()).
    target_compile_definitions(${target} PRIVATE ADHOC_INTERNAL=1)
endfunction()

function(adhoc_apply_platform_defines target)
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        target_compile_definitions(${target} PRIVATE ADHOC_MAC=1 ADHOC_WINDOWS=0)
    elseif(WIN32)
        target_compile_definitions(${target} PRIVATE
            ADHOC_MAC=0
            ADHOC_WINDOWS=1
            NOMINMAX
            WIN32_LEAN_AND_MEAN
            _CRT_SECURE_NO_WARNINGS
            UNICODE
            _UNICODE
        )
    endif()
endfunction()
