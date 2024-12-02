#include "Misc_Win.h"

#include <fmt/format.h>

#include <Enterprise/Core/PlatformData_Win.h>
#include <Enterprise/Core/PlatformHelpers_Win.h>

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <DbgHelp.h>

void Enterprise::PrintBacktrace_Implementation()
{
    auto platformState = Enterprise::PlatformData_Win::Get();

    const int maxFrames = 50;
    void* callStack[maxFrames];

    USHORT frames        = CaptureStackBackTrace(0, maxFrames, callStack, NULL);
    SYMBOL_INFO* symbol  = (SYMBOL_INFO*)malloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char));
    symbol->MaxNameLen   = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    fmt::print(stderr, "Backtrace:\n");
    for (USHORT i = 0; i < frames; ++i)
    {
        if (!SymFromAddr(platformState->processHandle, (DWORD64)(callStack[i]), 0, symbol))
        {
            fmt::print(stderr, " [{0}] SymFromAddr() failed! Error {1}", i, GetLastErrorAsString());
            continue;
        }

        fmt::print(stderr, " [{0}] (0x{1}) {2}\n", i, symbol->Address, symbol->Name);
    }

    free(symbol);
}
