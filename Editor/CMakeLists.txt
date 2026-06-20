find_package(fmt      CONFIG REQUIRED)
find_package(mimalloc CONFIG REQUIRED)

add_subdirectory(vendor)

# ---------------------------------------------------------------------------
# Editor source lists
# ---------------------------------------------------------------------------
set(EDITOR_SOURCES_COMMON
    src/Core/Internal/EditorMain.cpp
    src/Core/Internal/ImGuiSetup.cpp
    src/Views/ConsoleView.cpp
)

set(EDITOR_SOURCES_WINDOWS
    src/_platform/Windows/DllMain.cpp
    src/_platform/Windows/MimallocNewDeleteOverride.cpp
)

if(WIN32)
    set(EDITOR_SOURCES_PLATFORM ${EDITOR_SOURCES_WINDOWS})
else()
    set(EDITOR_SOURCES_PLATFORM)
endif()

# ---------------------------------------------------------------------------
# Editor shared library
# ---------------------------------------------------------------------------
add_library(Editor SHARED ${EDITOR_SOURCES_COMMON} ${EDITOR_SOURCES_PLATFORM})
target_include_directories(Editor
    PUBLIC
        "${CMAKE_SOURCE_DIR}/Editor/include"
    PRIVATE
        "${CMAKE_SOURCE_DIR}/Editor/src")
target_link_libraries(Editor PRIVATE mimalloc imgui_obj Engine)
if(APPLE)
    target_link_libraries(Editor PRIVATE
        "-framework AppKit"
        "-framework Metal"
        "-framework MetalKit"
        "-framework QuartzCore"
        "-framework GameController"
    )
endif()
target_compile_definitions(Editor PRIVATE
    IMGUI_USER_CONFIG="imuserconfig.h"
    ADHOC_EDITOR_PROJECT=1
)
set_target_properties(Editor PROPERTIES DEBUG_POSTFIX "D" DEV_POSTFIX "Dev")
adhoc_apply_target_defaults(Editor)

# Windows: generate per-target .rc from template.
if(WIN32)
    string(REPLACE "." "," ADHOC_VERSION_COMMA "${PROJECT_VERSION}")
    set(ADHOC_RC_FILENAME    "Editor.dll")
    set(ADHOC_RC_DESCRIPTION "Ad Hoc Editor DLL")
    set(ADHOC_RC_FILETYPE    "VFT_DLL")
    set(ADHOC_RC_ICON_ENTRY "")
    configure_file(
        "${CMAKE_SOURCE_DIR}/cmake/templates/AppResource.rc.in"
        "${CMAKE_BINARY_DIR}/generated/Editor.rc"
        @ONLY)
    target_sources(Editor PRIVATE
        "${CMAKE_BINARY_DIR}/generated/Editor.rc")
endif()
