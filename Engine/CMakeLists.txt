find_package(fmt         CONFIG REQUIRED)
find_package(magic_enum  CONFIG REQUIRED)
if(ADHOC_PLATFORM_DESKTOP)
    find_package(glfw3   CONFIG REQUIRED)
endif()

# ---------------------------------------------------------------------------
# Source lists
# ---------------------------------------------------------------------------
set(ENGINE_SOURCES_COMMON
    src/Core/Assertions.cpp
    src/Core/Console.cpp
    src/Core/Debugging.cpp
    src/Core/DynamicLibrary.cpp
    src/Core/Misc.cpp
    src/Core/RuntimeInfo.cpp
    src/Graphics/ApiMode.cpp
    src/Graphics/GraphicsContext.cpp
    src/Window/Internal/WindowSetup.cpp
    src/Window/WindowState.cpp
)

set(ENGINE_SOURCES_MAC
    src/Core/_platform/Mac/MacDebugging.cpp
    src/Core/_platform/Mac/MacDynamicLibrary.cpp
    src/Core/_platform/Mac/MacMisc.cpp
    src/Graphics/Contexts/_platform/Mac/MacMetalGraphicsContext.mm
    src/_platform/Mac/MetalCppImpl.cpp
)

set(ENGINE_SOURCES_WINDOWS
    src/Core/_platform/Windows/WindowsDebugging.cpp
    src/Core/_platform/Windows/WindowsDynamicLibrary.cpp
    src/Core/_platform/Windows/WindowsMisc.cpp
    src/Core/_platform/Windows/WindowsPlatformHelpers.cpp
    src/Core/_platform/Windows/WindowsRuntimeInfo.cpp
    src/Graphics/Contexts/_platform/Windows/WindowsD3D11GraphicsContext.cpp
    src/Window/Internal/_platform/Windows/WindowsSetMainWindowIcon.cpp
)

# DllMain is only for the shared (DLL) build on Windows.
set(ENGINE_SOURCES_WINDOWS_SHARED
    src/_platform/Windows/DllMain.cpp
)

if(APPLE)
    set(ENGINE_SOURCES_PLATFORM ${ENGINE_SOURCES_MAC})
elseif(WIN32)
    set(ENGINE_SOURCES_PLATFORM ${ENGINE_SOURCES_WINDOWS})
endif()

set(ENGINE_SOURCES_SHARED ${ENGINE_SOURCES_COMMON} ${ENGINE_SOURCES_PLATFORM})
if(WIN32)
    list(APPEND ENGINE_SOURCES_SHARED ${ENGINE_SOURCES_WINDOWS_SHARED})
endif()

set(ENGINE_SOURCES_STATIC ${ENGINE_SOURCES_COMMON} ${ENGINE_SOURCES_PLATFORM})

# fmt and magic_enum headers are used in Engine's public headers, so they
# must be PUBLIC so that consumers (Editor, Launcher, tests) get the include dirs.
set(_engine_public_libs fmt::fmt magic_enum::magic_enum)

# glfw and platform frameworks are only used in Engine's private sources.
set(_engine_private_libs)
if(ADHOC_PLATFORM_DESKTOP)
    list(APPEND _engine_private_libs glfw)
endif()
if(APPLE)
    list(APPEND _engine_private_libs
        "-framework Metal"
        "-framework MetalKit"
        "-framework Foundation"
        "-framework QuartzCore"
        "-framework AppKit"
    )
elseif(WIN32)
    list(APPEND _engine_private_libs
        dbghelp
        d3d11
    )
endif()

set(_engine_public_includes
    "${CMAKE_SOURCE_DIR}/Engine/include"
    "${CMAKE_SOURCE_DIR}/Engine/vendor/metal-cpp/include"
)
set(_engine_private_includes
    "${CMAKE_SOURCE_DIR}/Engine/src"
)

# ---------------------------------------------------------------------------
# Shared Engine library
# ---------------------------------------------------------------------------
if(ADHOC_PLATFORM_DESKTOP)
    add_library(Engine SHARED ${ENGINE_SOURCES_SHARED})
    target_include_directories(Engine
        PUBLIC  ${_engine_public_includes}
        PRIVATE ${_engine_private_includes})
    target_link_libraries(Engine
        PUBLIC  ${_engine_public_libs}
        PRIVATE ${_engine_private_libs})
    target_compile_definitions(Engine
        PUBLIC  ADHOC_EDITOR=1
        PRIVATE ADHOC_ENGINE_PROJECT=1)
    set_target_properties(Engine PROPERTIES DEBUG_POSTFIX "D" DEV_POSTFIX "Dev")
    adhoc_apply_target_defaults(Engine)

    if(APPLE)
        set_source_files_properties(
            src/Graphics/Contexts/_platform/Mac/MacMetalGraphicsContext.mm
            PROPERTIES COMPILE_OPTIONS "-fobjc-arc"
        )
    endif()

    if(WIN32)
        string(REPLACE "." "," ADHOC_VERSION_COMMA "${PROJECT_VERSION}")
        set(ADHOC_RC_FILENAME    "Engine.dll")
        set(ADHOC_RC_DESCRIPTION "Ad Hoc Engine DLL")
        set(ADHOC_RC_FILETYPE    "VFT_DLL")
        set(ADHOC_RC_ICON_ENTRY "")
        configure_file(
            "${CMAKE_SOURCE_DIR}/cmake/templates/AppResource.rc.in"
            "${CMAKE_BINARY_DIR}/generated/Engine.rc"
            @ONLY)
        target_sources(Engine PRIVATE
            "${CMAKE_BINARY_DIR}/generated/Engine.rc")
    endif()
endif()

# ---------------------------------------------------------------------------
# Static Engine library
# ---------------------------------------------------------------------------
add_library(EngineStatic STATIC ${ENGINE_SOURCES_STATIC})
set_target_properties(EngineStatic PROPERTIES POSITION_INDEPENDENT_CODE ON DEBUG_POSTFIX "D" DEV_POSTFIX "Dev")
target_include_directories(EngineStatic
    PUBLIC  ${_engine_public_includes}
    PRIVATE ${_engine_private_includes})
target_link_libraries(EngineStatic
    PUBLIC  ${_engine_public_libs}
    PRIVATE ${_engine_private_libs})
target_compile_definitions(EngineStatic
    PUBLIC  ADHOC_EDITOR=0
    PRIVATE ADHOC_ENGINE_PROJECT=0)
adhoc_apply_target_defaults(EngineStatic)

if(APPLE)
    set_source_files_properties(
        src/Graphics/Contexts/_platform/Mac/MacMetalGraphicsContext.mm
        PROPERTIES COMPILE_OPTIONS "-fobjc-arc"
    )
endif()
