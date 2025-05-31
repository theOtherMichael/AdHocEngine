#include <Engine/Window/WindowData.h>

namespace Engine::Window
{

const WindowData& WindowData::GetInstance()
{
    return GetMutableWindowData();
}

WindowData::~WindowData()
{
    mainWindowHandle = nullptr;
}

WindowData& GetMutableWindowData()
{
    static WindowData instance;
    return instance;
}

} // namespace Engine::Window
