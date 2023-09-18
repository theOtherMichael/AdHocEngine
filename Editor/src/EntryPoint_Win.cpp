#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

#include "ReloadSentinel_Win.h"

static const int lengthOfLoopInMilliseconds = 1000.0f;

static void Update()
{
#ifdef ENTERPRISE_DEBUG
    std::cout << "Debug..." << std::endl;
#elif defined(ENTERPRISE_DEV)
    std::cout << "Dev..." << std::endl;
#elif defined(ENTERPRISE_RELEASE)
    std::cout << "Release..." << std::endl;
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
            // 1. Dump Editor state (optionally with a prompt for the user)

            // 2. Tell the launcher to reload
            *reloadFlagsOut = currentReloadFlags;

            // 3. Return control to the launcher
            break;
        }
    }

    return EXIT_SUCCESS;
}
