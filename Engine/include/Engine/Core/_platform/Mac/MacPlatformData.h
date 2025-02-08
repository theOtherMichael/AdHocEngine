#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#include <GLFW/glfw3.h>
#pragma clang diagnostic pop

namespace Engine
{

#if ADHOC_INTERNAL
void InitializeProcessInfo() {};
void SetNativeWindowHandle(GLFWwindow* window);
#endif

} // namespace Engine
