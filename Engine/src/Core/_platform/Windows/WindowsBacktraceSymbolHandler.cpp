#include <Engine/Core/_platform/Windows/WindowsBacktraceSymbolHandler.h>

#include <Engine/Core/Assertions.h>
#include <Engine/Core/Console.h>
#include <Engine/Core/RuntimeState.h>
#include <Engine/Core/PlatformHelpers.h>

#include <DbgHelp.h>
#include <windows.h>

#include <filesystem>

ASSERT_PLATFORM_WINDOWS;

namespace fs = std::filesystem;

namespace Console = Engine::Console;

namespace Engine
{

WindowsBacktraceSymbolHandler::WindowsBacktraceSymbolHandler()
{
    const auto& platformData = Engine::RuntimeState::GetInstance();

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
    const auto& platformData = Engine::RuntimeState::GetInstance();

    Console::Log("Cleaning up symbol handler...");

    if (isValid && platformData.processHandle != NULL && !SymCleanup(platformData.processHandle))
    {
        Console::LogError("SymCleanup() failed! {}", Windows::GetLastErrorMessage());
    }
}

} // namespace Engine
