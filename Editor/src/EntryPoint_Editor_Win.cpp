#ifndef _WIN32
static_assert(false);
#endif //_WIN32

#include <atomic>
#include <chrono>
#include <filesystem>
#include <string>
#include <thread>

#include <fmt/format.h>
#include <GLFW/glfw3.h>

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

static void OnGlfwError(int error, const char* description)
{
    fmt::print(stderr, "GLFW error {}: {}", error, description);
}

bool InitSymbolHandler(bool isDevelopmentMode)
{
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

    bool isSymbolHandlerInitialized = false;
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

    return isSymbolHandlerInitialized;
}

void CleanUpSymbolHandler(const bool isSymbolHandlerInitialized)
{
    auto platformData = Enterprise::GetMutablePlatformData_Win();

    if (isSymbolHandlerInitialized && !SymCleanup(platformData->processHandle))
        fmt::print(stderr, "SymCleanup() failed! {}", GetLastErrorAsString());

    if (platformData->processHandle && !CloseHandle(platformData->processHandle))
        fmt::print(stderr, "Could not close process handle! {}", GetLastErrorAsString());

    platformData->processHandle = NULL;
}

extern "C" __declspec(dllexport) unsigned char EditorMain(int argc, char* argv[])
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

    bool isSymbolHandlerInitialized = InitSymbolHandler(isDevelopmentMode);

    static std::atomic_uchar reloadFlags = 0;
    if (isDevelopmentMode)
    {
        std::thread reloadWatchThread(WaitForEditorOrEngineRecompile, &reloadFlags);
        reloadWatchThread.detach();
    }

    glfwSetErrorCallback(OnGlfwError);

    if (!glfwInit())
    {
        CleanUpSymbolHandler(isSymbolHandlerInitialized);
        return EditorReloadFlag_None;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1024, 768, "Window Title", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        CleanUpSymbolHandler(isSymbolHandlerInitialized);
        return EditorReloadFlag_None;
    }

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        if (reloadFlags != EditorReloadFlag_None)
        {
            // Dump editor state here
            break;
        }
    }

    glfwTerminate();
    CleanUpSymbolHandler(isSymbolHandlerInitialized);
    return reloadFlags;
}
