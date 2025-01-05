#include <Engine/Core/_platform/Mac/MacMisc.h>

#include <Engine/Console.h>

#include <execinfo.h>

#ifndef ADHOC_MACOS
static_assert(false);
#endif

namespace Engine
{

void PrintBacktrace()
{
    const int maxFrames = 64;
    void* callstack[maxFrames];

    int frames     = backtrace(callstack, maxFrames);
    char** symbols = backtrace_symbols(callstack, frames);

    if (!symbols)
    {
        Console::LogError("Failed to retrieve stack trace symbols.");
        return;
    }

    Console::Log("Backtrace ({} frames):", frames);
    for (int i = 0; i < frames; ++i)
    {
        Console::Log(" {}", symbols[i]);
    }

    free(symbols);
}

} // namespace Engine
