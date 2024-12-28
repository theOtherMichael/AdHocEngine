#include <Engine/Core/_platform/Windows/WindowsMisc.h>

#include <Engine/Core/PlatformData.h>
#include <Engine/Core/PlatformHelpers.h>

#include <fmt/format.h>

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <DbgHelp.h>
#include <Windows.h>

#ifndef ADHOC_WINDOWS
static_assert(false);
#endif

namespace Engine
{

void PrintBacktrace()
{
    const int maxFrames = 64;
    void* callStack[maxFrames];

    USHORT frames        = CaptureStackBackTrace(0, maxFrames, callStack, NULL);
    SYMBOL_INFO* symbol  = (SYMBOL_INFO*)malloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char));
    symbol->MaxNameLen   = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    fmt::print("Backtrace ({} frames):\n", frames);

    const auto& platformData = PlatformData::GetInstance();

    for (USHORT i = 0; i < frames; ++i)
    {
        if (!SymFromAddr(platformData.processHandle, (DWORD64)(callStack[i]), 0, symbol))
        {
            fmt::print(stderr, " [{0}] SymFromAddr() failed! Error {1}\n", i, Windows::GetLastErrorMessage());
            continue;
        }

        fmt::print(stderr, " [{0}] (0x{1}) {2}\n", i, symbol->Address, symbol->Name);
    }

    free(symbol);
}

} // namespace Engine
