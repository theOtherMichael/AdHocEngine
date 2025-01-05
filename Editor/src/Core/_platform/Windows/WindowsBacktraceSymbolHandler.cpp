#include "WindowsBacktraceSymbolHandler.h"

#include <Engine/Console.h>
#include <Engine/Core/PlatformData.h>
#include <Engine/Core/PlatformHelpers.h>

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

namespace Console = Engine::Console;

namespace Editor
{

WindowsBacktraceSymbolHandler::WindowsBacktraceSymbolHandler(const bool isDeveloperMode)
{
    const auto& platformData = Engine::PlatformData::GetInstance();

    Console::Log("Initializing symbol handler...");

    if (!isDeveloperMode)
    {
        isValid = SymInitialize(platformData.processHandle, NULL, TRUE);
    }
    else
    {
        TCHAR rawPathToLauncher[MAX_PATH];
        if (!GetModuleFileName(NULL, rawPathToLauncher, MAX_PATH))
        {
            Console::LogError("Failed to get path to launcher!");
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
        Console::LogError(
            "SymInitialize() failed! {}\nBacktraces will be unavailable this session", Windows::GetLastErrorMessage());
    }
}

WindowsBacktraceSymbolHandler::~WindowsBacktraceSymbolHandler()
{
    const auto& platformData = Engine::PlatformData::GetInstance();

    Console::Log("Cleaning up symbol handler...");

    if (isValid && platformData.processHandle != NULL && !SymCleanup(platformData.processHandle))
    {
        Console::LogError("SymCleanup() failed! {}", Windows::GetLastErrorMessage());
    }
}

} // namespace Editor
