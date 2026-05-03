#include "MacMisc.h"

#include <Engine/Core/Assertions.h>
#include <Engine/Core/Console.h>

#include <fmt/format.h>

#include <dlfcn.h>
#include <execinfo.h>
#include <mach-o/dyld.h>

#include <filesystem>
#include <sstream>
#include <string>

static_assert(ADHOC_MAC);

namespace fs = std::filesystem;

namespace Engine::Platform
{

void* StackAllocImpl(size_t size)
{
    return alloca(size);
}

std::string GetBacktraceImpl()
{
    const int maxFrames = 64;
    void* callstack[maxFrames];

    auto frames  = backtrace(callstack, maxFrames);
    auto symbols = backtrace_symbols(callstack, frames);

    if (!symbols)
    {
        Console::LogError("Stack trace symbols are unavailable.");
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

fs::path GetExecutablePathImpl()
{
    constexpr auto maxPathBufferSize = PATH_MAX + 1;
    char rawPathToExecutable[maxPathBufferSize];

    uint32_t pathBufferLength = maxPathBufferSize;
    if (_NSGetExecutablePath(rawPathToExecutable, &pathBufferLength) != 0)
    {
        Console::LogError("Executable path is unavailable.");
        return fs::path();
    }

    char realRawPathToExecutable[maxPathBufferSize];
    if (realpath(rawPathToExecutable, realRawPathToExecutable) == NULL)
    {
        Console::LogError("Executable path could not be canonicalized.");
        return fs::path();
    }
    return realRawPathToExecutable;
}

} // namespace Engine::Platform
