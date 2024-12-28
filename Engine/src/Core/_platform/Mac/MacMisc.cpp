#include <Engine/Core/_platform/Mac/MacMisc.h>

#include <fmt/format.h>

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
        fmt::print("Failed to retrieve stack trace symbols.\n");
        return;
    }

    fmt::print("Backtrace ({} frames):\n", frames);
    for (int i = 0; i < frames; ++i)
    {
        fmt::print(" {}\n", symbols[i]);
    }

    free(symbols);
}

} // namespace Engine
