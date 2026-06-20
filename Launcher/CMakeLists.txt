find_package(mimalloc CONFIG REQUIRED)

# ---------------------------------------------------------------------------
# Launcher source lists
# ---------------------------------------------------------------------------
set(LAUNCHER_SOURCES_COMMON
    src/main.cpp
)

set(LAUNCHER_SOURCES_MAC
    src/_platform/Mac/MacMimallocInjectionImpl.cpp
)

set(LAUNCHER_SOURCES_WINDOWS
    src/_platform/Windows/MimallocNewDeleteOverride.cpp
)

if(APPLE)
    set(LAUNCHER_SOURCES_PLATFORM ${LAUNCHER_SOURCES_MAC})
elseif(WIN32)
    set(LAUNCHER_SOURCES_PLATFORM ${LAUNCHER_SOURCES_WINDOWS})
endif()

# Output name differs by platform. On Windows the per-config suffix comes from
# DEBUG_POSTFIX/DEV_POSTFIX below (postfixes apply to .exe targets). On macOS
# those postfixes are ignored for app-bundle executables, so the suffix is baked
# into OUTPUT_NAME here — making the built bundle AdHocEditor<suffix>.app with a
# matching inner Mach-O. The install step renames the bundle directory to the
# fixed "Ad Hoc Editor.app" (see cmake/AdHocMacBundle.cmake).
if(WIN32)
    set(_launcher_output_name "AdHocEditor")
else()
    set(_launcher_output_name "${ADHOC_BUNDLE_EXECUTABLE}")
endif()

# ---------------------------------------------------------------------------
# Launcher executable factory
# ---------------------------------------------------------------------------
# The Windows GUI Launcher (AdHocEditor.exe) and the console "terminal build"
# (AdHocEditorConsole.exe) are identical except for the output name, the
# subsystem (WIN32_EXECUTABLE), and the version-resource strings. Define both
# through one function so the source list, link libs, defaults, and the Windows
# .rc generation stay in lockstep. See docs/SourceMode.md for why the console
# twin exists (blocking, inherited stdio for CI/automation/source-mode).
function(adhoc_add_launcher target)
    cmake_parse_arguments(ARG "" "OUTPUT_NAME;WIN32_EXECUTABLE;RC_FILENAME;RC_DESCRIPTION" "" ${ARGN})

    add_executable(${target} ${LAUNCHER_SOURCES_COMMON} ${LAUNCHER_SOURCES_PLATFORM})
    set_target_properties(${target} PROPERTIES
        OUTPUT_NAME "${ARG_OUTPUT_NAME}"
        DEBUG_POSTFIX "D"
        DEV_POSTFIX "Dev"
    )
    if(WIN32)
        set_target_properties(${target} PROPERTIES WIN32_EXECUTABLE ${ARG_WIN32_EXECUTABLE})
        # mainCRTStartup is the console default and required to reach main() from
        # the GUI subsystem — explicit on both for clarity.
        target_link_options(${target} PRIVATE "/ENTRY:mainCRTStartup")
    endif()
    target_include_directories(${target} PRIVATE
        "${CMAKE_SOURCE_DIR}/Launcher/src"
        "${CMAKE_SOURCE_DIR}/Engine/include")
    target_link_libraries(${target} PRIVATE mimalloc Editor Engine)
    adhoc_apply_target_defaults(${target})

    # Windows: generate a per-target .rc from the shared template. The generated
    # path is keyed on the target name so the two launchers never share a file.
    if(WIN32)
        string(REPLACE "." "," ADHOC_VERSION_COMMA "${PROJECT_VERSION}")
        set(ADHOC_RC_FILENAME    "${ARG_RC_FILENAME}")
        set(ADHOC_RC_DESCRIPTION "${ARG_RC_DESCRIPTION}")
        set(ADHOC_RC_FILETYPE    "VFT_APP")
        set(ADHOC_RC_ICON_ENTRY
            "101 ICON \"${CMAKE_SOURCE_DIR}/Launcher/resources/Windows/AdHocEditor.ico\"")
        configure_file(
            "${CMAKE_SOURCE_DIR}/cmake/templates/AppResource.rc.in"
            "${CMAKE_BINARY_DIR}/generated/${target}.rc"
            @ONLY)
        target_sources(${target} PRIVATE
            "${CMAKE_BINARY_DIR}/generated/${target}.rc")
    endif()
endfunction()

# ---------------------------------------------------------------------------
# Launcher executables
# ---------------------------------------------------------------------------
# GUI subsystem launcher (all platforms).
adhoc_add_launcher(Launcher
    OUTPUT_NAME      "${_launcher_output_name}"
    WIN32_EXECUTABLE ON
    RC_FILENAME      "${_launcher_output_name}.exe"
    RC_DESCRIPTION   "Ad Hoc Editor")

# Console subsystem "terminal build" — Windows only. WIN32_EXECUTABLE OFF yields
# /SUBSYSTEM:CONSOLE, so it blocks until exit and routes stdout/stderr to the
# caller's console or pipe (CI/automation/source-mode). macOS/Linux already
# behave this way with the single editor binary, so no twin is needed there.
if(WIN32)
    adhoc_add_launcher(LauncherConsole
        OUTPUT_NAME      "${_launcher_output_name}Console"
        WIN32_EXECUTABLE OFF
        RC_FILENAME      "${_launcher_output_name}Console.exe"
        RC_DESCRIPTION   "Ad Hoc Editor (Console)")
endif()
