#include "MacMimallocInjectionImpl.h"

#include <Engine/Core/Console.h>
#include <Engine/Core/Misc.h>

#include <mach-o/dyld.h>
#include <unistd.h>

#include <cerrno>
#include <cstdint>
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

#if ADHOC_DEBUG
constexpr auto MimallocLibName = "libmimalloc-debug.2.2.dylib";
constexpr auto BundleSubdir    = "debug";
#else
constexpr auto MimallocLibName     = "libmimalloc.2.2.dylib";
constexpr const char* BundleSubdir = nullptr;
#endif

constexpr auto SentinelEnvVar = "ADHOC_MIMALLOC_REEXEC";

bool IsMimallocLoaded()
{
    const auto count = _dyld_image_count();
    for (uint32_t i = 0; i < count; ++i)
    {
        const auto* name = _dyld_get_image_name(i);
        if (name == nullptr)
            continue;

        if (std::string_view(name).find("libmimalloc") != std::string_view::npos)
            return true;
    }
    return false;
}

fs::path ResolveBundleDylibPath(const fs::path& exePath)
{
    auto current = exePath.parent_path();
    while (!current.empty() && current != current.parent_path())
    {
        if (current.extension() == ".app")
        {
            auto frameworks = current / "Contents" / "Frameworks";
            if (BundleSubdir != nullptr)
                frameworks /= BundleSubdir;
            return frameworks / MimallocLibName;
        }
        current = current.parent_path();
    }
    return fs::path();
}

} // namespace

namespace Platform
{

void EnsureMimallocInjected(int /*argc*/, char* argv[])
{
    if (IsMimallocLoaded())
    {
        unsetenv(SentinelEnvVar);
        return;
    }

    if (std::getenv(SentinelEnvVar) != nullptr)
    {
        unsetenv(SentinelEnvVar);
        Console::LogError("mi-malloc injection failed after re-exec; "
                          "the dylib was silently rejected by dyld.");
        return;
    }

    const auto exePath = Engine::GetExecutablePath();
    if (exePath.empty())
        return;

    const auto dylibPath = ResolveBundleDylibPath(exePath);
    if (dylibPath.empty())
    {
        Console::LogError("mi-malloc is not loaded and no .app ancestor was found to locate it. "
                          "Configure the run scheme with DYLD_INSERT_LIBRARIES, "
                          "or run from a built .app bundle.");
        return;
    }

    auto ec = std::error_code();
    if (!fs::exists(dylibPath, ec))
    {
        Console::LogError("mi-malloc dylib was not found at expected bundle path: {}", dylibPath.string());
        return;
    }

    setenv("DYLD_INSERT_LIBRARIES", dylibPath.c_str(), 1);
    setenv(SentinelEnvVar, "1", 1);

    execv(exePath.c_str(), argv);

    const auto savedErrno = errno;
    unsetenv(SentinelEnvVar);
    Console::LogError(
        "mi-malloc injection failed: execv returned with errno={} ({}).", savedErrno, std::strerror(savedErrno));
}

} // namespace Platform
