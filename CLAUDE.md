# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Building

**Windows (Visual Studio 2022):** Open `AdHocEngine.sln`.
- `Debug` / `Dev` / `Release` — build and run the editor (engine and editor as dynamic libraries)
- `StaticDebug` / `StaticDev` / `StaticRelease` — build the engine as a static library for games to link against

Dynamic configs also trigger their corresponding Static build as a post-build step, since the static engine lib is shipped alongside the editor for use by game projects. vcpkg dependencies install automatically on build.

**macOS (Xcode):** Open `AdHocEngine.xcworkspace`. Same configuration names apply, plus a `Bundle` scheme for shipping.

## Tests

Tests use Google Test. To run a specific test, use gtest's `--gtest_filter` flag on the built executable. Use `python scripts/run_tests.py` to run suites from the command line.

- **EngineTests** — build and run in any configuration.
- **EditorTests** — build and run in `Debug` / `Dev` / `Release` only. The editor is only ever built as a DLL, so EditorTests do not exist in the `Static*` configurations.

Mirror the source folder structure when adding test files: tests for `Engine/include/FolderA/ClassA.h` go in `EngineTests/src/FolderA/ClassATests.cpp`. Do not put tests in anonymous namespaces (Visual Studio won't surface inline controls for them).

## Code Formatting

clang-format is enforced — never commit unformatted code.
- **Visual Studio**: Ctrl+K, Ctrl+D (document) or Ctrl+K, Ctrl+F (selection)
- **Entire repo**: `python scripts/clang_format.py --format`
- **Check only**: `python scripts/clang_format.py --check`

Use `// clang-format off` / `// clang-format on` only when clang-format produces genuinely undesirable results (e.g., macro alignment).

## Xcode Sync (when developing on Windows)

Run `ruby scripts/update_xcode_sources.rb` regularly to keep Xcode projects in sync with Visual Studio changes.

## Architecture

The engine is split into four Visual Studio/Xcode projects:

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
- vcpkg (`vcpkg.json` per project) — preferred for all new dependencies. After adding a package, link its output from `vcpkg_installed/` manually in the project settings. Engine uses `fmt`, `magic-enum`, and `glfw3`.
- `vendor/` — git submodules or source drops when vcpkg isn't viable. Keep the dependency's license in its folder.
- On macOS, link from the `arm64-osx-{dynamic|static}-adhoc` triplet folders inside `vcpkg_installed/`.

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
- `Expect_*` — non-fatal, logs but doesn't throw (for developer-mode tolerance only)
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
