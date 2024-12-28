#include "MacPlatformMisc.h"

#include <dlfcn.h>
#include <mach-o/dyld.h>

#include <filesystem>
#include <iostream>

#ifndef ADHOC_MACOS
static_assert(false);
#endif

namespace fs = std::filesystem;

namespace Platform
{

fs::path GetLauncherPath()
{
    constexpr auto maxPathBufferSize = PATH_MAX + 1;
    char cPathToLauncher[maxPathBufferSize];

    uint32_t pathBufferLength = maxPathBufferSize;
    if (_NSGetExecutablePath(cPathToLauncher, &pathBufferLength) != 0)
    {
        std::cerr << "Failed to get path to launcher!\n";
        return fs::path();
    }

    char realCPathToLauncher[maxPathBufferSize];
    if (realpath(cPathToLauncher, realCPathToLauncher) == NULL)
    {
        std::cerr << "Failed to resolve real path to launcher!\n";
        return fs::path();
    }
    return realCPathToLauncher;
}

} // namespace Platform
