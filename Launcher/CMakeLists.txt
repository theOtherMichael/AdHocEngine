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
# Launcher executable
# ---------------------------------------------------------------------------
add_executable(Launcher ${LAUNCHER_SOURCES_COMMON} ${LAUNCHER_SOURCES_PLATFORM})
set_target_properties(Launcher PROPERTIES
    OUTPUT_NAME "${_launcher_output_name}"
    DEBUG_POSTFIX "D"
    DEV_POSTFIX "Dev"
)
target_include_directories(Launcher PRIVATE
    "${CMAKE_SOURCE_DIR}/Launcher/src"
    "${CMAKE_SOURCE_DIR}/Engine/include")
target_link_libraries(Launcher PRIVATE Editor Engine mimalloc)
adhoc_apply_target_defaults(Launcher)

# Windows: generate per-target .rc from template.
if(WIN32)
    string(REPLACE "." "," ADHOC_VERSION_COMMA "${PROJECT_VERSION}")
    set(ADHOC_RC_FILENAME    "AdHocEditor.exe")
    set(ADHOC_RC_DESCRIPTION "Ad Hoc Editor")
    set(ADHOC_RC_FILETYPE    "VFT_APP")
    set(ADHOC_RC_ICON_ENTRY
        "IDI_ICON1 ICON \"${CMAKE_SOURCE_DIR}/Launcher/resources/Windows/AdHocEditor.ico\"")
    configure_file(
        "${CMAKE_SOURCE_DIR}/cmake/templates/AppResource.rc.in"
        "${CMAKE_BINARY_DIR}/generated/Launcher.rc"
        @ONLY)
    target_sources(Launcher PRIVATE
        "${CMAKE_BINARY_DIR}/generated/Launcher.rc")
endif()
