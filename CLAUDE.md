# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Building

The build system is CMake 3.28+ with single-config Ninja. Each `(platform, config)` pair is a separate preset and lives in its own build tree — `build/host-mac-debug/`, `build/host-mac-dev/`, `build/host-mac-release/`, and so on for Windows and cross-compile targets. Switching configs means switching presets, not flipping a config selector.

**Prerequisites (automatic):** vcpkg dependencies install on first configure. The pinned LLVM toolchain downloads automatically via `scripts/bootstrap_llvm.py` if not present.

**macOS:**
```sh
cmake --preset host-mac-debug
cmake --build build/host-mac-debug                       # builds Engine, EngineStatic, Editor, Launcher
cmake --build build/host-mac-debug --target Engine       # build one target
cmake --build build/host-mac-release --target Launcher
```

**Windows:**
```sh
cmake --preset host-windows-debug
cmake --build build/host-windows-debug
```

**Cross-compile (engine-static only):**
```sh
cmake --preset cross-ios-arm64-release
cmake --build build/cross-ios-arm64-release
```

**Multiple trees at once** — use `scripts/build.py` which configures + builds presets in parallel:
```sh
python scripts/build.py                                  # default: host-<platform>-debug
python scripts/build.py host-mac-dev cross-ios-arm64-dev
```

**Source-mode layout (for running the editor from outside the build tree, e.g. for the future hot-reload flow):**
```sh
cmake --build build/host-mac-debug --target source-mode   # populates out/source/a/
```

**Shipping bundle / installer:**
```sh
python scripts/package.py                                # builds all shipped configs, merges, codesigns
```
The shipping bundle merges Debug + Release first-party binaries (Engine + EngineD, etc.) into a single bundle/installer along with the embedded LLVM toolchain and cross-compiled static engine libs.

**IDE IntelliSense after adding a source file:** Sources are listed explicitly in `CMakeLists.txt` (no globbing), so a new `.cpp` needs a CMake *configure* before IntelliSense resolves it — until then cpptools falls back to default flags and shows spurious errors (`ADHOC_MAC` undefined, missing `std::array` CTAD, etc.). CMake Tools reconfigures automatically when you save the `CMakeLists.txt` edit in VS Code (`cmake.configureOnEdit`, pinned in `.vscode/settings.json`). If a file was added or `CMakeLists.txt` was edited *outside* the IDE (CLI build, agent edit), the running extension isn't notified — run **CMake: Configure** from the Command Palette, or reload the window, to refresh it.

## Tests

Tests use Google Test. Each tree is single-config, so ctest doesn't need `-C`:
```sh
ctest --test-dir build/host-mac-debug --output-on-failure
```
To run a specific test, use gtest's `--gtest_filter` flag on the built executable. Use `python scripts/test.py [preset]` to run suites from the command line.

- **EngineTests / EngineTestsStatic** — built in every tree. `EngineTests` links the shared `Engine`; `EngineTestsStatic` links `EngineStatic`.
- **EditorTests** — built in host trees only (Editor has no static variant).

Mirror the source folder structure when adding test files: tests for `Engine/include/FolderA/ClassA.h` go in `EngineTests/src/FolderA/ClassATests.cpp`. Do not put tests in anonymous namespaces (Visual Studio won't surface inline controls for them).

## Code Formatting

clang-format is enforced — never commit unformatted code.
- **Check only**: `python scripts/format.py`
- **Apply formatting**: `python scripts/format.py --apply`

Use `// clang-format off` / `// clang-format on` only when clang-format produces genuinely undesirable results (e.g., macro alignment).

## Architecture

The engine is split into four CMake subdirectories:

| Project | Role |
|---|---|
| `Engine` | Core library — window, graphics, assertions, console, ASL |
| `Editor` | Editor application built on top of Engine, uses Dear ImGui |
| `Launcher` | Standalone game launcher executable |
| `EngineTests` / `EditorTests` | Google Test suites (link against static engine) |

**Engine subsystems** (under `Engine/include/Engine/` and `Engine/src/`):

| Folder | Contents |
|---|---|
| `Core/` | Assertions, Console, Debugging, DynamicLibrary, Misc, PlatformAbstraction, RuntimeInfo, SymbolExportMacros |
| `Core/Formatters/` | `EnumFormatter.h` — `INJECT_ENUM_FORMATTER` macro; adds `fmt` + `magic_enum` formatting for enums in a namespace |
| `Common/` | `BorrowHandle<T>` (non-owning pointer with assert), `ThreadSafeViews` (`ThreadSafeSharedView`, `ThreadSafeExclusiveView`, async variants), `Singleton<T>`, `PlatformHelpers` |
| `Window/` | `WindowState.h` (public), `Window/Internal/WindowSetup.h` (internal setup) |
| `Graphics/` | Graphics context abstraction and per-API implementations |

**Ad Hoc Support Library (ASL)** lives in `Engine/include/asl/`. It has no engine dependencies and uses `snake_case` to mirror the standard library. Use it for general-purpose C++ utilities; put engine-specific reusables in `Engine/Common/` instead.

| Header | Contents |
|---|---|
| `asl/finally.h` | `asl::finally` — RAII scope-exit guard |
| `asl/casts.h` | `asl::narrow_cast<T>` (unchecked), `asl::narrow<T>` (throws `asl::narrowing_error` on truncation) |
| `asl/concepts.h` | `asl::arithmetic`, `asl::non_arithmetic` concepts |

**Platform Abstraction Layer (PAL):** All platform-specific code lives in `_platform/` subfolders. Generic code pulls in platform implementations via two macros from `<Engine/Core/PlatformAbstraction.h>`:

```cpp
// Required platform implementation — compile error if missing
#include PLATFORM_HEADER(MyFunctionImpl.h)

// Optional platform implementation — provide fallback if missing
#if PLATFORM_HEADER_EXISTS(OptionalImpl.h)
#include PLATFORM_HEADER(OptionalImpl.h)
#define PLATFORM_SUPPORTS_X 1
#else
#define PLATFORM_SUPPORTS_X 0
#endif
```

`PLATFORM_HEADER(Foo.h)` expands to `"_platform/<Platform>/<Platform>Foo.h"`. All platform implementations go in the `Platform::` namespace. Never add symbols to `Platform::` in generic code. Add `static_assert(ADHOC_WINDOWS);` (or `ADHOC_MAC`) near the top of every platform-specific `.cpp` file.

When abstracting an entire class to the platform layer, use a `using` alias or pimpl. For classes of meaningful complexity, define a C++20 concept for the contract and `static_assert` that the platform implementation satisfies it.

**Graphics:** `BaseGraphicsContext` interface with per-API subclasses (D3D11, D3D12, Metal, OpenGL, Vulkan). Use concept-based type checking (e.g., `IsD3d11GraphicsContext`) rather than dynamic dispatch where possible.

**Dependencies:**
- vcpkg (root `vcpkg.json`) — preferred for all new dependencies. Features: `editor` (glfw3, mimalloc), `tests` (gtest). Add a package to `vcpkg.json`, then add a `find_package` + `target_link_libraries` call in the appropriate `CMakeLists.txt`. Engine uses `fmt`, `magic-enum`, and `glfw3`.
- `vendor/` — git submodules or source drops when vcpkg isn't viable. Keep the dependency's license in its folder.
- Custom vcpkg triplets live in `triplets/`. On macOS, the dynamic triplet is `arm64-osx-dynamic-adhoc`; static is `arm64-osx-static-adhoc`.

## Naming & Style

| Element | Convention |
|---|---|
| Types, public members, functions | `PascalCase` |
| Private members, parameters, locals | `camelCase` |
| Macros | `SCREAMING_SNAKE_CASE` |
| ASL types & functions | `snake_case` |

- Engine code → `::Engine`, editor code → `::Editor`. Internal APIs → `::Engine::Feature::Internal`. Platform implementations → `Platform::` sub-namespace. Max three levels of nesting (Internal/Platform don't count against this).
- Use **structs** for types with no private members; **classes** otherwise. Explicit `private:` always. Access order: `public` → `protected` → `private`.
- Follow AAA (Almost Always Auto) style. Use literal suffixes or brace-init to make types explicit when needed.
- Use explicit lambda capture lists (`[varName]` or `[&varName]`). Never use blanket `[=]` or `[&]`.
- Use `#pragma once`. Include what you use directly — no relying on transitive includes.

## Assertions & Error Handling

Include `<Engine/Core/Assertions.h>`. Assertion families:
- `Assert_*` — fatal in Debug/Dev (throws `Engine::AssertionFailedException`)
- `Expect_*` — non-fatal, logs but doesn't throw (use when an assertion failure need not crash the process)
- `Eval_*` — expression evaluated even in Release; logs in Debug/Dev
- `*_Slow` variants — stripped from Dev and Release (use in hot paths)
- `*_Fmt` variants — accept a custom message (omit when the assertion is already self-documenting)

Prefer `static_assert` over runtime assertions. Prefer `std::expected` (or `asl::unexpected`) over exceptions for fallible functions. Exceptions are enabled but should not leak from functions that may be called in game builds.

All errors go through `<Engine/Core/Console.h>` (`Console::LogError`, etc.) so they appear in the editor's Console view and log files.

**Error message style:** Present tense, passive voice. End with a period unless the message ends with a `{}` argument. Sentence case only. Include relevant runtime values via `{fmt}` arguments. Do not manually include function name or file path — the assertion library captures call site info automatically.

## File Organization

- Source files → `src/`, exported headers → `include/`, private headers → `src/`
- One header per `.cpp` file (not both public and private for the same source)
- All filenames must be unique across the project (except public/private header pairs)
- Non-code assets → `resources/`
- Never put first-party files in `vendor/` or `vcpkg_installed/`
- Folder categories: `Core/` (foundational), `Common/` (reusable utilities), `Views/` (editor panels), then per-subsystem folders (`Window/`, `Graphics/`, etc.)
