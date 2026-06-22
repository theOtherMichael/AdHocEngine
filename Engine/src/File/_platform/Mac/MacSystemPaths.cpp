#include "MacSystemPaths.h"

#include <Engine/Core/Console.h>

#include <mach-o/dyld.h>

#include <filesystem>

static_assert(ADHOC_MAC);

namespace fs = std::filesystem;

namespace Engine::Platform
{

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
