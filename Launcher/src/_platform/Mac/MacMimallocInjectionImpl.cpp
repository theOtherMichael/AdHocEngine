// macOS mimalloc override via DYLD_INSERT_LIBRARIES.
//
// EnsureMimallocInjected() makes mimalloc the process allocator however the app
// is launched. On macOS mimalloc overrides malloc/free only when its dylib is
// loaded via DYLD_INSERT_LIBRARIES; *linking* it (the editor does, for its API)
// does NOT override the allocator. So this keys off that injection variable, and
// when it is not yet in effect:
//   * Launched directly, no debugger -> set DYLD_INSERT_LIBRARIES to the bundled
//     dylib and re-exec once. The variable is then in effect for the child, so it
//     does not re-exec again.
//   * Launched under a debugger without a preload -> do NOT re-exec; fall back to
//     the system allocator for this run. Re-exec'ing a *traced* process with
//     DYLD_INSERT_LIBRARIES set faults inside dyld on macOS 26 (EXC_BAD_ACCESS in
//     dyld4::ExternallyViewableState, then SIGKILL). The source-mode launch
//     config preloads the dylib so injection is already in effect and mimalloc
//     stays active while debugging; see .vscode/launch.json plus the
//     libmimalloc-adhoc symlink staged by cmake/AdHocMacBundle.cmake.
//
// The .app entitlements grant com.apple.security.cs.allow-dyld-environment-
// variables (and disable library validation) so injection works under the
// hardened runtime.
//
// KNOWN BENIGN WARNING — do not chase it:
//   mimalloc: error: mi_free: invalid pointer: 0x...
// Fired exactly once, before main(). mimalloc's macOS override interposes the
// `free` symbol globally, but a few bytes get allocated during dyld/libSystem/
// runtime bring-up *before* mimalloc's initializer registers — and when that
// bootstrap pointer is later freed it reaches mi_free, which (unlike the malloc
// zone dispatch) won't delegate a foreign pointer back to the system allocator,
// so it logs and skips it. mimalloc is still the process-wide allocator (verify
// with MIMALLOC_VERBOSE=1 / MIMALLOC_SHOW_STATS=1); this is a one-time ~32-byte
// leak inherent to dynamic override on macOS, present in the old Xcode setup
// too. The only true fix is a zone-only mimalloc build (MI_OSX_INTERPOSE=OFF),
// which we deliberately don't maintain.

#include "MacMimallocInjectionImpl.h"

#include <Engine/Core/Console.h>
#include <Engine/Core/Debugging.h>
#include <Engine/Core/Misc.h>

#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string_view>
#include <system_error>

static_assert(ADHOC_MAC);

namespace fs      = std::filesystem;
namespace Console = Engine::Console;

namespace
{

// Use the unversioned symlink (e.g. libmimalloc-debug.dylib -> ...3.3.dylib) so
// this survives vcpkg mimalloc version bumps. The SourceMode install step stages
// each config's vcpkg dylibs under Contents/Frameworks/ -- Dev/Release flat, but
// Debug under Frameworks/debug/ so like-named debug/release dylibs coexist after
// the shipping merge (see docs/BuildLayout.md). The Frameworks subpath must match
// the config, or ResolveBundleDylibPath looks in the wrong place and injection
// silently falls back to the system allocator.
#if ADHOC_DEBUG
constexpr auto MimallocLibName   = "libmimalloc-debug.dylib";
constexpr auto FrameworksSubpath = "Frameworks/debug";
#else
constexpr auto MimallocLibName   = "libmimalloc.dylib";
constexpr auto FrameworksSubpath = "Frameworks";
#endif

// True when mimalloc is interposing malloc/free for this process. On macOS that
// holds only when its dylib was loaded via DYLD_INSERT_LIBRARIES — having the
// dylib merely linked/loaded (the editor links it for its API) does NOT override
// the allocator, so checking the image list would be a false positive. The
// injection variable is the real signal; it is set by the re-exec below or by
// the launch config's preload, and survives the re-exec so the child sees it.
bool IsMimallocInjected()
{
    const auto* inserted = std::getenv("DYLD_INSERT_LIBRARIES");
    return inserted != nullptr && std::string_view(inserted).find("libmimalloc") != std::string_view::npos;
}

fs::path ResolveBundleDylibPath(const fs::path& exePath)
{
    auto current = exePath.parent_path();
    while (!current.empty() && current != current.parent_path())
    {
        if (current.extension() == ".app")
            return current / "Contents" / FrameworksSubpath / MimallocLibName;
        current = current.parent_path();
    }
    return fs::path();
}

} // namespace

namespace Platform
{

void EnsureMimallocInjected(int /*argc*/, char* argv[])
{
    // Already overriding the allocator — our own re-exec below set
    // DYLD_INSERT_LIBRARIES, or the launch config preloaded the dylib. Done.
    if (IsMimallocInjected())
    {
        Console::Log("mi-malloc injected successfully.");
        return;
    }

    // Under a debugger we must not re-exec to inject: re-exec'ing a traced
    // process with DYLD_INSERT_LIBRARIES set faults inside dyld on macOS 26
    // (EXC_BAD_ACCESS in dyld4::ExternallyViewableState, then SIGKILL). Preload
    // the dylib via DYLD_INSERT_LIBRARIES in the launch configuration to debug
    // with mimalloc (the source-mode launch config does); otherwise this run uses the
    // system allocator.
    if (Engine::IsBeingDebugged())
    {
        Console::LogWarning("mi-malloc is not preloaded under the debugger; continuing with the "
                            "system allocator. Preload it via DYLD_INSERT_LIBRARIES in the launch "
                            "configuration to debug with mimalloc.");
        return;
    }

    const auto exePath = Engine::GetExecutablePath();
    if (exePath.empty())
        return;

    const auto dylibPath = ResolveBundleDylibPath(exePath);
    if (dylibPath.empty())
    {
        Console::LogError("mi-malloc is not injected and no .app ancestor was found to locate the dylib. "
                          "Set DYLD_INSERT_LIBRARIES, or run from a built .app bundle.");
        return;
    }

    auto ec = std::error_code();
    if (!fs::exists(dylibPath, ec))
    {
        Console::LogError("mi-malloc dylib was not found at expected bundle path: {}", dylibPath.string());
        return;
    }

    // Re-exec with the variable set; the child sees IsMimallocInjected() and runs
    // through. execv only returns on failure.
    setenv("DYLD_INSERT_LIBRARIES", dylibPath.c_str(), 1);
    execv(exePath.c_str(), argv);

    const auto savedErrno = errno;
    Console::LogError(
        "mi-malloc injection failed: execv returned with errno={} ({}).", savedErrno, std::strerror(savedErrno));
}

} // namespace Platform
