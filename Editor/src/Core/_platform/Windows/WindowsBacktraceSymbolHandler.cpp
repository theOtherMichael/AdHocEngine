#include "WindowsBacktraceSymbolHandler.h"

#include <Engine/Core/PlatformData.h>
#include <Engine/Core/PlatformHelpers.h>

#include <fmt/format.h>

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <DbgHelp.h>
#include <Windows.h>

#include <filesystem>

#ifndef ADHOC_WINDOWS
static_assert(false);
#endif

namespace fs = std::filesystem;

namespace Editor
{

WindowsBacktraceSymbolHandler::WindowsBacktraceSymbolHandler(const bool isDeveloperMode)
{
    const auto& platformData = Engine::PlatformData::GetInstance();

    fmt::print("Initializing symbol handler...\n");

    if (!isDeveloperMode)
    {
        isValid = SymInitialize(platformData.processHandle, NULL, TRUE);
    }
    else
    {
        TCHAR rawPathToLauncher[MAX_PATH];
        if (!GetModuleFileName(NULL, rawPathToLauncher, MAX_PATH))
        {
            fmt::print(stderr, "Failed to get path to launcher!\n");
            isValid = false;
        }
        else
        {
            fs::path pathToLauncher    = rawPathToLauncher;
            fs::path pathToBuildFolder = pathToLauncher.parent_path().parent_path();
            fs::path pathToReloadCache = pathToBuildFolder / "Launcher\\reload_cache";

            isValid = SymInitialize(platformData.processHandle, pathToReloadCache.string().c_str(), TRUE);
        }
    }

    if (!isValid)
    {
        fmt::print(stderr,
                   "SymInitialize() failed! {}\nBacktraces will be unavailable this session\n",
                   Windows::GetLastErrorMessage());
    }
}

WindowsBacktraceSymbolHandler::~WindowsBacktraceSymbolHandler()
{
    const auto& platformData = Engine::PlatformData::GetInstance();

    fmt::print("Cleaning up symbol handler...\n");

    if (isValid && platformData.processHandle != NULL && !SymCleanup(platformData.processHandle))
    {
        fmt::print(stderr, "SymCleanup() failed! {}\n", Windows::GetLastErrorMessage());
    }
}

} // namespace Editor
