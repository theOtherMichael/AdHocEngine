#include <Engine/Window/WindowState.h>

namespace Engine::Window
{

const WindowState& WindowState::GetInstance()
{
    return GetMutableWindowState();
}

WindowState::~WindowState()
{
    mainWindowHandle = nullptr;
}

WindowState& GetMutableWindowState()
{
    static WindowState instance;
    return instance;
}

} // namespace Engine::Window
