#ifndef _WIN32
static_assert(false);
#endif //_WIN32

#include <atomic>
#include <chrono>
#include <filesystem>
#include <string>
#include <thread>

#include <fmt/format.h>

#include <Enterprise/Core/PlatformHelpers_Win.h>
#include <Enterprise/Core/PlatformData_Win.h>

#include <Editor/EditorReloadFlags.h>
#include "ReloadSentinel_Win.h"

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <DbgHelp.h>

namespace fs = std::filesystem;

static const int lengthOfLoopInMilliseconds = 1000.0f;

static void Update()
{
#ifdef ENTERPRISE_DEBUG
    fmt::print("Debug...\n");
#elif defined(ENTERPRISE_DEV)
    fmt::print("Dev...\n");
#elif defined(ENTERPRISE_RELEASE)
    fmt::print("Release...\n");
#endif

    std::this_thread::sleep_for(std::chrono::milliseconds(lengthOfLoopInMilliseconds));
}

extern "C" __declspec(dllexport) int EditorMain(int argc,
                                                char* argv[],
                                                unsigned char* reloadFlagsOut)
{
    const char* editorReloadableFlag = "--development";
    bool isDevelopmentMode           = false;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], editorReloadableFlag) == 0)
        {
            isDevelopmentMode = true;
            break;
        }
    }

    static std::atomic_uchar currentReloadFlags = 0;

    if (isDevelopmentMode)
    {
        std::thread watcherThread(WaitForEditorOrEngineRecompile, &currentReloadFlags);
        watcherThread.detach();
    }

    auto platformData = Enterprise::GetMutablePlatformData_Win();
    if (!DuplicateHandle(GetCurrentProcess(),
                         GetCurrentProcess(),
                         GetCurrentProcess(),
                         &(platformData->processHandle),
                         0,
                         FALSE,
                         DUPLICATE_SAME_ACCESS))
    {
        fmt::print(stderr, "Could not obtain process handle! {}", GetLastErrorAsString());
    }

    BOOL isSymbolHandlerInitialized = FALSE;
    if (isDevelopmentMode)
    {
#ifdef ENTERPRISE_DEBUG
        std::string symbolFileSearchPath = fs::canonical("build\\Debug\\").string();
#elif defined(ENTERPRISE_DEV)
        std::string symbolFileSearchPath = fs::canonical("build\\Dev\\").string();
#elif defined(ENTERPRISE_RELEASE)
        std::string symbolFileSearchPath = fs::canonical("build\\Release\\").string();
#else
        static_assert(false);
#endif // ENTERPRISE_DEBUG

        isSymbolHandlerInitialized = SymInitialize(
            platformData->processHandle, symbolFileSearchPath.c_str(), TRUE);
    }
    else
    {
        isSymbolHandlerInitialized =
            SymInitialize(platformData->processHandle, NULL, TRUE);
    }

    if (!isSymbolHandlerInitialized)
    {
        fmt::print(
            stderr,
            "SymInitialize() failed! {}\nBacktraces will be unavailable this session",
            GetLastErrorAsString());
    }

    while (true)
    {
        Update();

        if (currentReloadFlags != EditorReloadFlag_None)
        {
            // Dump editor state here

            *reloadFlagsOut = currentReloadFlags;
            break;
        }
    }

    if (isSymbolHandlerInitialized && !SymCleanup(platformData->processHandle))
        fmt::print(stderr, "SymCleanup() failed! {}", GetLastErrorAsString());

    if (platformData->processHandle && !CloseHandle(platformData->processHandle))
        fmt::print(stderr, "Could not close process handle! {}", GetLastErrorAsString());

    platformData->processHandle = NULL;

    return EXIT_SUCCESS;
}
