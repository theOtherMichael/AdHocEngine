cmake_minimum_required(VERSION 3.28)

# ---------------------------------------------------------------------------
# 1. LLVM bootstrap — must happen before project() so the compiler is locked.
# ---------------------------------------------------------------------------
include(cmake/AdHocLLVMVersion.cmake)
include(cmake/AdHocLLVMToolchain.cmake)
adhoc_bootstrap_llvm()

# ---------------------------------------------------------------------------
# 2. vcpkg bootstrap + toolchain — must happen before project().
# ---------------------------------------------------------------------------
include(cmake/AdHocVcpkg.cmake)
adhoc_bootstrap_vcpkg()

set(VCPKG_OVERLAY_TRIPLETS "${CMAKE_SOURCE_DIR}/triplets" CACHE STRING "")
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE
        "${CMAKE_SOURCE_DIR}/tools/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE FILEPATH "")
endif()

# ---------------------------------------------------------------------------
# 3. Single-config build type setup. Each build tree picks one of Debug/Dev/Release.
# All optimization and define flags are applied per-target by AdHocCompilerFlags
# and AdHocDefines; the per-config flag variables are cleared so our helpers own
# every flag.
# ---------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (Debug, Dev, Release)" FORCE)
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Dev Release)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug" AND
   NOT CMAKE_BUILD_TYPE STREQUAL "Dev"   AND
   NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    message(FATAL_ERROR
        "CMAKE_BUILD_TYPE must be one of Debug / Dev / Release (got '${CMAKE_BUILD_TYPE}').")
endif()

# Config-derived names/paths — the single source of truth consumed by the
# Launcher target, the install scripts, and the generated Info.plist. See
# docs/BuildLayout.md. First-party binaries carry a per-config suffix
# (Debug -> D, Dev -> Dev, Release -> none); Debug artifacts are isolated in a
# "debug/" subfolder so like-named debug/release dependencies coexist.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(ADHOC_CONFIG_SUFFIX "D")
    set(ADHOC_DEBUG_SUBDIR  "debug")
elseif(CMAKE_BUILD_TYPE STREQUAL "Dev")
    set(ADHOC_CONFIG_SUFFIX "Dev")
    set(ADHOC_DEBUG_SUBDIR  "")
else()
    set(ADHOC_CONFIG_SUFFIX "")
    set(ADHOC_DEBUG_SUBDIR  "")
endif()
set(ADHOC_BUNDLE_EXECUTABLE "AdHocEditor${ADHOC_CONFIG_SUFFIX}")

# ---------------------------------------------------------------------------
# 4. Project declaration.
# ---------------------------------------------------------------------------
project(AdHocEngine VERSION 0.1.0.1 LANGUAGES C CXX)
if(APPLE)
    enable_language(OBJCXX)
endif()

# Clear the per-config flag variables so our helpers own all flags.
# CMAKE_<LANG>_FLAGS_DEV / CMAKE_<LINKER>_FLAGS_DEV need to exist so CMake
# accepts Dev as a valid build type for single-config generators.
foreach(_lang IN ITEMS C CXX OBJCXX)
    foreach(_cfg IN ITEMS DEBUG DEV RELEASE)
        set(CMAKE_${_lang}_FLAGS_${_cfg} "" CACHE STRING "" FORCE)
    endforeach()
endforeach()
foreach(_linker IN ITEMS EXE_LINKER SHARED_LINKER MODULE_LINKER STATIC_LINKER)
    foreach(_cfg IN ITEMS DEBUG DEV RELEASE)
        set(CMAKE_${_linker}_FLAGS_${_cfg} "" CACHE STRING "" FORCE)
    endforeach()
endforeach()

# ---------------------------------------------------------------------------
# 5. Toolchain enforcement — hard-reject anything that isn't upstream clang.
# ---------------------------------------------------------------------------
if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    message(FATAL_ERROR
        "AppleClang is not supported. Use upstream LLVM clang from "
        "tools/llvm/${ADHOC_LLVM_VERSION}. Run cmake --preset host-mac-debug.")
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(FATAL_ERROR
        "MSVC is not supported. Use upstream LLVM clang-cl from "
        "tools/llvm/${ADHOC_LLVM_VERSION}. Run cmake --preset host-windows-debug.")
endif()
if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(FATAL_ERROR
        "Only upstream LLVM Clang is supported (got ${CMAKE_CXX_COMPILER_ID}).")
endif()
if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL ADHOC_LLVM_VERSION)
    message(FATAL_ERROR
        "Clang ${CMAKE_CXX_COMPILER_VERSION} does not match the pinned version "
        "${ADHOC_LLVM_VERSION}.")
endif()
message(STATUS "Clang OK — ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# ---------------------------------------------------------------------------
# 6. Single-source version propagation.
# ---------------------------------------------------------------------------
configure_file(
    cmake/templates/Version.h.in
    "${CMAKE_SOURCE_DIR}/Engine/include/Engine/Core/Version.h"
    @ONLY)
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    configure_file(
        cmake/templates/Info.plist.in
        "${CMAKE_BINARY_DIR}/generated/Info.plist"
        @ONLY)
endif()

# ---------------------------------------------------------------------------
# 7. CMake modules.
# ---------------------------------------------------------------------------
# Dev is not a standard CMake config, so imported targets from vcpkg would
# otherwise fall back to their Debug variant. Map it to Release so Dev builds
# link the optimized vcpkg libs.
set(CMAKE_MAP_IMPORTED_CONFIG_DEV "Release")

include(cmake/AdHocTargetDefaults.cmake)

# Block CPack in every tree: the shipping artifact is produced by
# scripts/package.py, which merges multiple (platform, config) trees. A single
# tree can only ever produce a broken installer, so cpack / `--target package`
# abort with a redirect to package.py.
include(cmake/AdHocBlockCPack.cmake)
adhoc_block_cpack()

# ---------------------------------------------------------------------------
# 8. Projects.
# ---------------------------------------------------------------------------
# Short-circuit for cross-compile preset: only build the static engine.
if(NOT ADHOC_PLATFORM_DESKTOP)
    add_subdirectory(Engine)
    return()
endif()

add_subdirectory(Engine)
add_subdirectory(Editor)
add_subdirectory(Launcher)

include(CTest) # calls enable_testing() and generates DartConfiguration.tcl for `ctest -T test`
add_subdirectory(EngineTests)
add_subdirectory(EditorTests)

# ---------------------------------------------------------------------------
# 9. Platform-specific packaging / install rules.
# ---------------------------------------------------------------------------
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    include(cmake/AdHocMacBundle.cmake)
    adhoc_configure_mac_install()
elseif(WIN32)
    include(cmake/AdHocWindowsInstaller.cmake)
    adhoc_configure_windows_install()
endif()
