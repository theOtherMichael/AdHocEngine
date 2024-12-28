#include "WindowsPlatformMisc.h"

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include <filesystem>
#include <iostream>

#ifndef ADHOC_WINDOWS
static_assert(false);
#endif

namespace fs = std::filesystem;

namespace Platform
{

fs::path GetLauncherPath()
{
    TCHAR cPathToLauncher[MAX_PATH];
    if (!GetModuleFileName(NULL, cPathToLauncher, MAX_PATH))
    {
        std::cerr << "Failed to get path to launcher!\n";
        return fs::path();
    }
    return cPathToLauncher;
}

} // namespace Platform
