#include <Engine/Core/_platform/Windows/WindowsMisc.h>

#include <Engine/Console.h>
#include <Engine/Core/PlatformData.h>
#include <Engine/Core/PlatformHelpers.h>

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

    Console::Log("Backtrace ({} frames):", frames);

    const auto& platformData = PlatformData::GetInstance();

    for (USHORT i = 0; i < frames; ++i)
    {
        if (!SymFromAddr(platformData.processHandle, (DWORD64)(callStack[i]), 0, symbol))
        {
            Console::LogError(" [{0}] SymFromAddr() failed! Error {1}", i, Windows::GetLastErrorMessage());
            continue;
        }

        Console::LogError(" [{0}] (0x{1}) {2}", i, symbol->Address, symbol->Name);
    }

    free(symbol);
}

} // namespace Engine
