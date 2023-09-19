#include <atomic>
#include <chrono>
#include <thread>

#include <fmt/format.h>

#include "ReloadSentinel_Win.h"

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
            isDevelopmentMode = true;
    }

    static std::atomic_uchar currentReloadFlags = 0;

    if (isDevelopmentMode)
    {
        std::thread watcherThread(Editor::WaitForDirChange, &currentReloadFlags);
        watcherThread.detach();
    }

    while (true)
    {
        Update();

        if (currentReloadFlags != ReloadFlag_None)
        {
            // Dump editor state here

            *reloadFlagsOut = currentReloadFlags;
            break;
        }
    }

    return EXIT_SUCCESS;
}
