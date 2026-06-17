include(cmake/AdHocCompilerFlags.cmake)
include(cmake/AdHocDefines.cmake)

# Applies the full set of project-standard flags and defines to a target.
# Call this on every first-party target after declaring it.
function(adhoc_apply_target_defaults target)
    adhoc_apply_compiler_flags(${target})
    adhoc_apply_variant_flags(${target})
    adhoc_apply_config_defines(${target})
    adhoc_apply_platform_defines(${target})
endfunction()
