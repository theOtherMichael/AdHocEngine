#pragma once

#include <Engine/Core/SymbolExportMacros.h>

#include <Windows.h>

#include <GLFW/glfw3.h>

namespace Engine::Window::Windows
{

ENGINE_API void SetWindowIconWithEmbeddedIconResource(GLFWwindow* window, int resourceId);

} // namespace Engine::Window::Windows
