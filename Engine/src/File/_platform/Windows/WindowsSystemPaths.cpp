#include "WindowsSystemPaths.h"

#include <Engine/Common/PlatformHelpers.h>
#include <Engine/Core/Console.h>

#include <windows.h>

#include <array>
#include <filesystem>
#include <system_error>

static_assert(ADHOC_WINDOWS);

namespace fs = std::filesystem;

namespace Engine::Platform
{

fs::path GetExecutablePathImpl()
{
    auto rawPath = std::array<TCHAR, MAX_PATH>{};
    if (!GetModuleFileName(NULL, rawPath.data(), MAX_PATH))
    {
        Console::LogError("Executable path is unavailable. Error: {}", GetLastErrorMessage());
        return fs::path{};
    }
    const auto verbatimPath = fs::path{rawPath.data()};

    auto canonicalizationError = std::error_code{};
    auto canonicalPath         = fs::canonical(verbatimPath, canonicalizationError);

    if (canonicalizationError)
    {
        Console::LogError("Executable path could not be canonicalized. Error: {}", canonicalizationError.message());
        return verbatimPath;
    }

    return canonicalPath;
}

} // namespace Engine::Platform
