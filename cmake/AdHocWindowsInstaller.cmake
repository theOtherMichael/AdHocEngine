# Configures install() rules that produce a fully-launchable runtime layout
# for the active build tree's single config under the install prefix.
#
# Usage:
#   cmake --install build/<preset> --component SourceMode --prefix out/source/a/
#
# Multi-config shipping (merging per-config trees, then makensis) is handled by
# scripts/package.py.
#
function(adhoc_configure_windows_install)
    cmake_policy(SET CMP0177 NEW)
    # Debug isolates exe + DLLs + PDBs under a "debug/" subfolder so like-named
    # debug/release vcpkg DLLs coexist in the merged shipping tree, and so the
    # debug exe sits beside its own DLLs (Windows resolves DLLs from the loading
    # module's directory). Dev/Release stay at the top level. See docs/BuildLayout.md.
    if(ADHOC_DEBUG_SUBDIR)
        set(_dest "${ADHOC_DEBUG_SUBDIR}")
    else()
        set(_dest ".")
    endif()

    install(TARGETS Launcher Editor Engine
        RUNTIME DESTINATION "${_dest}" COMPONENT SourceMode
        LIBRARY DESTINATION "${_dest}" COMPONENT SourceMode
        ARCHIVE DESTINATION "${_dest}/lib" COMPONENT SourceMode
    )

    # vcpkg-installed runtime DLLs.
    install(
        FILES $<TARGET_RUNTIME_DLLS:Launcher>
        DESTINATION "${_dest}"
        COMPONENT SourceMode
    )

    # mimalloc-redirect.dll is a peer companion that mimalloc.dll loads to intercept
    # allocations across DLL boundaries. It is not a CMake link target, so
    # TARGET_RUNTIME_DLLS misses it; install it explicitly from the vcpkg bin dir.
    if(ADHOC_DEBUG_SUBDIR)
        set(_vcpkg_bin "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/debug/bin")
    else()
        set(_vcpkg_bin "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/bin")
    endif()
    install(
        FILES "${_vcpkg_bin}/mimalloc-redirect.dll"
        DESTINATION "${_dest}"
        COMPONENT SourceMode
    )

    # Debug symbols — required for the cold-start debug session to resolve
    # symbols, since the editor runs from a copy (a slot), not from build/.
    # clang-cl emits PDBs, so TARGET_PDB_FILE resolves; OPTIONAL guards configs
    # that don't produce one.
    install(
        FILES
            $<TARGET_PDB_FILE:Launcher>
            $<TARGET_PDB_FILE:Editor>
            $<TARGET_PDB_FILE:Engine>
        DESTINATION "${_dest}"
        COMPONENT SourceMode
        OPTIONAL
    )

    # Convenience target (cmake --build build/<preset> --target source-mode) that
    # stages the active tree's layout into the global slot A (out/source/a/); see
    # docs/SourceMode.md for the slot model.
    add_custom_target(source-mode
        # Wipe the slot first so files orphaned by a layout/config change (e.g. a
        # stale debug/ subfolder) don't linger.
        COMMAND "${CMAKE_COMMAND}" -E rm -rf "${CMAKE_SOURCE_DIR}/out/source/a"
        COMMAND "${CMAKE_COMMAND}"
            --install "${CMAKE_BINARY_DIR}"
            --component SourceMode
            --prefix   "${CMAKE_SOURCE_DIR}/out/source/a"
        DEPENDS Launcher Engine Editor
        USES_TERMINAL
        COMMENT "Installing SourceMode layout to out/source/a/"
    )
endfunction()
