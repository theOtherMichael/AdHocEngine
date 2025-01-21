#include <Engine/Core/_platform/Windows/WindowsBacktraceSymbolHandler.h>

#include <Engine/Core/Console.h>
#include <Engine/Core/PlatformData.h>
#include <Engine/Core/PlatformHelpers.h>

#include <DbgHelp.h>
#include <windows.h>

#include <filesystem>

#if !ADHOC_WINDOWS
static_assert(false);
#endif

namespace fs = std::filesystem;

namespace Console = Engine::Console;

namespace Engine
{

WindowsBacktraceSymbolHandler::WindowsBacktraceSymbolHandler()
{
    const auto& platformData = Engine::PlatformData::GetInstance();

    Console::Log("Initializing symbol handler...");

    isValid = SymInitialize(platformData.processHandle, NULL, TRUE);

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

} // namespace Engine
