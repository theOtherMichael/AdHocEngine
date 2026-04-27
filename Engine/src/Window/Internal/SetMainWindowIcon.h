#pragma once

#include <Engine/Core/PlatformAbstraction.h>

#if PLATFORM_HEADER_EXISTS(SetMainWindowIcon.h)
#include PLATFORM_HEADER(SetMainWindowIcon.h)
#define PLATFORM_SUPPORTS_MAIN_WINDOW_ICON 1
#else
#define PLATFORM_SUPPORTS_MAIN_WINDOW_ICON 0
#endif

namespace Engine::Window::Internal
{

inline void SetMainWindowIcon()
{
#if PLATFORM_SUPPORTS_MAIN_WINDOW_ICON
    Platform::SetMainWindowIconImplementation();
#endif
}

} // namespace Engine::Window::Internal
