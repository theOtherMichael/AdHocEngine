#include "Core/BacktraceSymbolHandler.h"
#include <Editor/Core/EditorReloadFlags.h>
#include <Editor/Core/SymbolExportMacros.h>

#include <Engine/Core/Assertions.h>
#include <Engine/Core/PlatformData.h>

#include <fmt/format.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include <GLFW/glfw3.h>
#pragma clang diagnostic pop

#include <filesystem>

namespace fs = std::filesystem;

static void OnGlfwError(int error, const char* description)
{
    fmt::print(stderr, "GLFW error {}: {}\n", error, description);
}

extern "C" EDITOR_API unsigned char EditorMain(int argc, char* argv[])
{
    bool isDeveloperMode = false;
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--developer") == 0)
        {
            isDeveloperMode = true;
            break;
        }
    }

    Engine::InitializePlatformData();

    auto symbolHandler = Editor::BacktraceSymbolHandler(isDeveloperMode);

    // TODO: Reimplement the recompile watch thread

    glfwSetErrorCallback(OnGlfwError);

    if (!glfwInit())
    {
        return EditorReloadFlag_None;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* mainWindowPtr = glfwCreateWindow(1024, 768, "Window Title", nullptr, nullptr);

    if (!mainWindowPtr)
    {
        glfwTerminate();
        return EditorReloadFlag_None;
    }

    while (!glfwWindowShouldClose(mainWindowPtr))
    {
        glfwWaitEvents();

        // TODO: Check recompile watch thread
    }

    glfwTerminate();

    return EditorReloadFlag_None;
}
