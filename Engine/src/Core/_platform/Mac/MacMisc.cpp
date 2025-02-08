#include <Engine/Core/_platform/Mac/MacMisc.h>

#include <Engine/Core/Console.h>

#include <fmt/format.h>

#include <dlfcn.h>
#include <execinfo.h>
#include <mach-o/dyld.h>

#include <filesystem>
#include <sstream>
#include <string>

#if !ADHOC_MACOS
static_assert(false);
#endif

namespace fs = std::filesystem;

namespace Engine
{

fs::path GetExecutablePath()
{
    constexpr auto maxPathBufferSize = PATH_MAX + 1;
    char rawPathToExecutable[maxPathBufferSize];

    uint32_t pathBufferLength = maxPathBufferSize;
    if (_NSGetExecutablePath(rawPathToExecutable, &pathBufferLength) != 0)
    {
        Console::LogError("Failed to get path to launcher!");
        return fs::path();
    }

    char realRawPathToExecutable[maxPathBufferSize];
    if (realpath(rawPathToExecutable, realRawPathToExecutable) == NULL)
    {
        Console::LogError("Failed to resolve real path to launcher!");
        return fs::path();
    }
    return realRawPathToExecutable;
}

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
