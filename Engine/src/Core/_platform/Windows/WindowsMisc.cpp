#include <Engine/Core/_platform/Windows/WindowsMisc.h>

#include <Engine/Core/Assertions.h>
#include <Engine/Core/Console.h>
#include <Engine/Core/PlatformData.h>
#include <Engine/Core/PlatformHelpers.h>

#include <fmt/format.h>

#include <DbgHelp.h>
#include <windows.h>

#include <sstream>
#include <string>

ASSERT_PLATFORM_WINDOWS;

namespace fs = std::filesystem;

namespace Engine
{

fs::path GetExecutablePath()
{
    TCHAR rawPathToExecutable[MAX_PATH];
    if (!GetModuleFileName(NULL, rawPathToExecutable, MAX_PATH))
    {
        Console::LogError("Failed to get path to executable! Error: {}", Windows::GetLastErrorMessage());
        return fs::path();
    }
    return rawPathToExecutable;
}

std::string GetBacktrace()
{
    const int maxFrames = 64;
    void* callStack[maxFrames];

    auto frames          = CaptureStackBackTrace(0, maxFrames, callStack, NULL);
    auto symbol          = (SYMBOL_INFO*)malloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char));
    symbol->MaxNameLen   = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    const auto& platformData = PlatformData::GetInstance();

    auto output = std::ostringstream{};
    for (auto i = 0; i < frames; ++i)
    {
        if (!SymFromAddr(platformData.processHandle, (DWORD64)(callStack[i]), 0, symbol))
        {
            output << fmt::format("[{}] SymFromAddr() failed! Error {}\n", i, Windows::GetLastErrorMessage());
            continue;
        }

        output << fmt::format("[{}] (0x{}) {}\n", i, symbol->Address, symbol->Name);
    }

    free(symbol);

    return output.str();
}

} // namespace Engine
