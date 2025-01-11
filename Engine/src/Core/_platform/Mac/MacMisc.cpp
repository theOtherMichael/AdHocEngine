#include <Engine/Core/_platform/Mac/MacMisc.h>

#include <Engine/Core/Console.h>

#include <fmt/format.h>

#include <execinfo.h>

#include <sstream>
#include <string>

#ifndef ADHOC_MACOS
static_assert(false);
#endif

namespace Engine
{

std::string GetBacktrace()
{
    const int maxFrames = 64;
    void* callstack[maxFrames];

    auto frames  = backtrace(callstack, maxFrames);
    auto symbols = backtrace_symbols(callstack, frames);

    if (!symbols)
    {
        Console::LogError("Failed to retrieve stack trace symbols.");
        return std::string();
    }

    auto output = std::ostringstream();
    for (int i = 0; i < frames; ++i)
    {
        output << fmt::format("{}\n", symbols[i]);
    }

    free(symbols);

    return output.str();
}

} // namespace Engine
