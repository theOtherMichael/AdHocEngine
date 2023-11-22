#if !(defined(__APPLE__) && defined(__MACH__))
static_assert(false);
#endif // !(defined(__APPLE__) && defined(__MACH))

#include <fmt/format.h>
#include <GLFW/glfw3.h>

#include <Editor/EditorReloadFlags.h>

static void OnGlfwError(int error, const char* description)
{
    fmt::print(stderr, "GLFW error {}: {}\n", error, description);
}

extern "C" unsigned char EditorMain(int argc, char* argv[])
{
    glfwSetErrorCallback(OnGlfwError);
    
    if (!glfwInit())
        return EditorReloadFlag_None;
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* mainWindowPtr =
        glfwCreateWindow(1024, 768, "Window Title", nullptr, nullptr);
    
    if (!mainWindowPtr)
    {
        glfwTerminate();
        return EditorReloadFlag_None;
    }
    
    while (!glfwWindowShouldClose(mainWindowPtr))
    {
        glfwWaitEvents();
    }

    glfwTerminate();
    return EditorReloadFlag_None;
}
