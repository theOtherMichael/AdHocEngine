#if !(defined(__APPLE__) && defined(__MACH__))
static_assert(false);
#endif // !(defined(__APPLE__) && defined(__MACH))

#include <fmt/format.h>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include <GLFW/glfw3.h>
#pragma clang diagnostic pop

#include <Editor/Core/SymbolExportMacros.h>
#include <Editor/Core/EditorReloadFlags.h>

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

    std::atomic_uchar reloadFlags = EditorReloadFlag_None;

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

        if (reloadFlags != EditorReloadFlag_None)
        {
            // Dump editor state here
            break;
        }
    }

    glfwTerminate();
    return EditorReloadFlag_None;
}
