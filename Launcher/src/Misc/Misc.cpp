#include <iostream>
#include <filesystem>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif // WIN32_LEAN_AND_MEAN
    #include <Windows.h>
#elif defined(__APPLE__) && defined(__MACH__)
    #include <dlfcn.h>
    #include <mach-o/dyld.h>
#else
static_assert(false);
#endif

namespace fs = std::filesystem;

namespace Misc
{

fs::path GetLauncherPath()
{
#ifdef _WIN32
    TCHAR cPathToLauncher[MAX_PATH];
    if (!GetModuleFileName(NULL, cPathToLauncher, MAX_PATH))
    {
        std::cerr << "Failed to get path to launcher!\n";
        return fs::path();
    }
    return cPathToLauncher;

#elif defined(__APPLE__) && defined(__MACH__)
    constexpr auto maxPathBufferSize = PATH_MAX + 1;
    char cPathToLauncher[maxPathBufferSize];

    uint32_t pathBufferLength = maxPathBufferSize;
    if (_NSGetExecutablePath(cPathToLauncher, &pathBufferLength) != 0)
    {
        std::cerr << "Failed to get path to launcher!\n";
        return fs::path();
    }

    char realCPathToLauncher[maxPathBufferSize];
    if (realpath(cPathToLauncher, realCPathToLauncher) == NULL)
    {
        std::cerr << "Failed to resolve real path to launcher!\n";
        return fs::path();
    }
    return realCPathToLauncher;

#else
    static_assert(false);

#endif
}

} // namespace Misc
