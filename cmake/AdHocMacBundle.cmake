# Configures install() rules that assemble one fully-launchable "Ad Hoc Editor.app"
# for the active build tree's single config.  Called from the top-level
# CMakeLists after all subdirectories are included.
#
# What this produces under the install prefix (suffix: D=Debug, Dev, none=Release;
# Debug isolates its dependencies under Frameworks/debug — see docs/BuildLayout.md):
#   Ad Hoc Editor.app/Contents/MacOS/AdHocEditor<suffix>
#   Ad Hoc Editor.app/Contents/Info.plist        (CFBundleExecutable = AdHocEditor<suffix>)
#   Ad Hoc Editor.app/Contents/Frameworks[/debug]/{libEngine,libEditor,...}.dylib
#   Ad Hoc Editor.app/Contents/Resources/AppIcon.icns
#   Ad Hoc Editor.app/Contents/Resources/Assets.car
#   plus rpath fixups and adhoc codesign
#
# Usage:
#   cmake --install build/<preset> --component SourceMode --prefix out/source/a/
#
# Multi-config shipping (merging per-config trees) is handled by scripts/package.py.
#
# Cache variable: ADHOC_SIGNING_IDENTITY (default "-" = adhoc)
#
function(adhoc_configure_mac_install)
    set(ADHOC_SIGNING_IDENTITY "-" CACHE STRING "codesign identity (- = adhoc)")

    # Compile Assets.xcassets → AppIcon.icns + Assets.car via actool.
    _adhoc_prepare_app_icon()

    # Launcher becomes a .app bundle.
    set_target_properties(Launcher PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME          "Ad Hoc Editor"
        MACOSX_BUNDLE_GUI_IDENTIFIER       "dev.adhocengine.editor"
        MACOSX_BUNDLE_BUNDLE_VERSION       "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_INFO_PLIST           "${CMAKE_BINARY_DIR}/generated/Info.plist"
    )

    install(TARGETS Launcher
        BUNDLE DESTINATION .
        COMPONENT SourceMode
    )

    # The bundle is *built* as AdHocEditor<suffix>.app (OUTPUT_NAME); install
    # rules target that name, and the post-install step below renames the
    # directory to the fixed "Ad Hoc Editor.app". Debug dependencies are isolated
    # under Frameworks/debug so like-named debug/release vcpkg dylibs coexist in
    # the merged shipping bundle; the executable reaches them via its rpath.
    set(_built_bundle    "AdHocEditor${ADHOC_CONFIG_SUFFIX}.app")
    set(_bundle_contents "${_built_bundle}/Contents")
    set(_fw_subpath "Frameworks")
    if(ADHOC_DEBUG_SUBDIR)
        set(_fw_subpath "Frameworks/${ADHOC_DEBUG_SUBDIR}")
    endif()

    install(FILES
            "${CMAKE_BINARY_DIR}/generated/AppIcon.icns"
            "${CMAKE_BINARY_DIR}/generated/Assets.car"
        DESTINATION "${_bundle_contents}/Resources"
        COMPONENT SourceMode
    )

    # Engine + Editor dylibs.
    install(TARGETS Engine Editor
        LIBRARY DESTINATION "${_bundle_contents}/${_fw_subpath}"
        COMPONENT SourceMode
    )

    # vcpkg dylibs (exclude test libraries and CMake metadata dirs).
    # Pick the config-matched source variant: Debug pulls the debug vcpkg libs
    # under debug/lib; Dev/Release pull the release libs under lib. They install
    # into ${_fw_subpath} (Frameworks/debug for Debug), so when scripts/package.py
    # merges a Debug + Release tree the two variants land in separate folders and
    # coexist even when they share a filename.
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(_vcpkg_lib_subdir "debug/lib")
    else()
        set(_vcpkg_lib_subdir "lib")
    endif()
    install(
        DIRECTORY "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/${_vcpkg_lib_subdir}/"
        DESTINATION "${_bundle_contents}/${_fw_subpath}"
        COMPONENT SourceMode
        FILES_MATCHING
        PATTERN  "pkgconfig"   EXCLUDE
        PATTERN  "manual-link" EXCLUDE
        REGEX    "libgtest"    EXCLUDE
        REGEX    "libgmock"    EXCLUDE
        PATTERN  "*.dylib"
    )

    # Post-install: rename the built bundle to the fixed display name, then
    # rpath fixup + adhoc codesign for the single-config bundle.
    install(CODE
        "
        set(_install_root \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}\")
        set(_built_path \"\${_install_root}/${_built_bundle}\")
        set(_app_path   \"\${_install_root}/Ad Hoc Editor.app\")
        if(NOT _built_path STREQUAL _app_path)
            file(REMOVE_RECURSE \"\${_app_path}\")
            file(RENAME \"\${_built_path}\" \"\${_app_path}\")
        endif()

        set(_macos_dir \"\${_app_path}/Contents/MacOS\")
        set(_fw_dir    \"\${_app_path}/Contents/${_fw_subpath}\")

        # Fix each dylib: canonical @rpath install name + @loader_path rpath for siblings.
        file(GLOB _dylibs \"\${_fw_dir}/*.dylib\")
        foreach(_lib IN LISTS _dylibs)
            get_filename_component(_name \"\${_lib}\" NAME)
            execute_process(COMMAND install_name_tool -id \"@rpath/\${_name}\" \"\${_lib}\")
            execute_process(COMMAND install_name_tool
                -add_rpath \"@loader_path\" \"\${_lib}\"
                ERROR_QUIET)
            foreach(_other IN LISTS _dylibs)
                execute_process(COMMAND install_name_tool
                    -change \"\${_lib}\" \"@rpath/\${_name}\" \"\${_other}\"
                    ERROR_QUIET)
            endforeach()
        endforeach()

        # Add Frameworks rpath to each MacOS binary so @rpath dependencies resolve.
        # Points at the config's Frameworks subfolder (Frameworks/debug for Debug).
        file(GLOB _binaries \"\${_macos_dir}/*\")
        foreach(_bin IN LISTS _binaries)
            execute_process(COMMAND install_name_tool
                -add_rpath \"@loader_path/../${_fw_subpath}\" \"\${_bin}\"
                ERROR_QUIET)
        endforeach()

        execute_process(COMMAND codesign
            --force --deep
            --options runtime
            --entitlements \"${CMAKE_SOURCE_DIR}/cmake/codesign/editor.entitlements\"
            --sign \"${ADHOC_SIGNING_IDENTITY}\"
            \"\${_app_path}\"
            RESULT_VARIABLE _cs_result)
        if(NOT _cs_result EQUAL 0)
            message(WARNING \"codesign returned \${_cs_result} — bundle may not be launchable.\")
        endif()
        "
        COMPONENT SourceMode
    )

    # Stable, config-agnostic alias for the mimalloc variant this config stages
    # (Debug ships libmimalloc-debug.dylib; Dev/Release ship libmimalloc.dylib).
    # The source-mode launch config points DYLD_INSERT_LIBRARIES at this fixed
    # path so mimalloc is preloaded under the debugger and the Launcher never has
    # to re-exec to inject it — re-exec'ing a traced process crashes dyld on
    # macOS 26. Lives in the slot root (outside the signed .app) so adding it
    # neither touches the bundle signature nor leaks into shipping bundles.
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(_mi_dylib "libmimalloc-debug.dylib")
    else()
        set(_mi_dylib "libmimalloc.dylib")
    endif()
    set(_mi_alias "${CMAKE_SOURCE_DIR}/out/source/libmimalloc-adhoc.dylib")

    # Convenience target (cmake --build build/<preset> --target source-mode) that
    # stages the active tree's bundle into the global slot A (out/source/a/); see
    # docs/SourceMode.md for the slot model.
    add_custom_target(source-mode
        # Wipe the slot first so files orphaned by a layout/config change (e.g. a
        # stale Frameworks/debug, or an old bundle name) don't linger.
        COMMAND "${CMAKE_COMMAND}" -E rm -rf "${CMAKE_SOURCE_DIR}/out/source/a"
        COMMAND "${CMAKE_COMMAND}"
            --install "${CMAKE_BINARY_DIR}"
            --component SourceMode
            --prefix   "${CMAKE_SOURCE_DIR}/out/source/a"
        COMMAND "${CMAKE_COMMAND}" -E rm -f "${_mi_alias}"
        COMMAND "${CMAKE_COMMAND}" -E create_symlink
            "a/Ad Hoc Editor.app/Contents/${_fw_subpath}/${_mi_dylib}" "${_mi_alias}"
        DEPENDS Launcher Engine Editor
        USES_TERMINAL
        COMMENT "Installing SourceMode layout to out/source/a/"
    )
endfunction()

# ---------------------------------------------------------------------------
# Internal: compile Assets.xcassets → AppIcon.icns + Assets.car via actool.
#
# actool is the only supported path. Full Xcode.app is required — Command Line
# Tools alone does not ship actool.
# ---------------------------------------------------------------------------
function(_adhoc_prepare_app_icon)
    set(_assets "${CMAKE_SOURCE_DIR}/Launcher/resources/Mac/Assets.xcassets")
    set(_compile_dir "${CMAKE_BINARY_DIR}/generated")
    set(_icns_out "${_compile_dir}/AppIcon.icns")
    set(_car_out  "${_compile_dir}/Assets.car")

    find_program(ACTOOL_EXECUTABLE actool
        HINTS /Applications/Xcode.app/Contents/Developer/usr/bin)

    if(NOT ACTOOL_EXECUTABLE)
        message(FATAL_ERROR
            "actool not found. Install full Xcode.app (Command Line Tools alone "
            "does not ship actool) and run: "
            "sudo xcode-select -s /Applications/Xcode.app/Contents/Developer")
    endif()

    add_custom_command(
        OUTPUT "${_icns_out}" "${_car_out}"
        COMMAND "${ACTOOL_EXECUTABLE}"
            --output-format human-readable-text
            --notices --warnings
            --platform macosx
            --minimum-deployment-target 11.5
            --target-device mac
            --app-icon AppIcon
            --include-all-app-icons
            --compress-pngs
            --output-partial-info-plist "${_compile_dir}/actool_partial.plist"
            --compile "${_compile_dir}"
            "${_assets}"
        DEPENDS "${_assets}"
        COMMENT "Compiling asset catalog (actool)"
        VERBATIM
    )

    add_custom_target(AdHocAppIcon ALL DEPENDS "${_icns_out}" "${_car_out}")
    add_dependencies(Launcher AdHocAppIcon)
endfunction()
