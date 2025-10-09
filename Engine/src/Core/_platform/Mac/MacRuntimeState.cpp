#include <Engine/Core/_platform/Mac/MacRuntimeState.h>

#include <Engine/Core/Assertions.h>

ASSERT_PLATFORM_MACOS;

namespace Engine
{

const MacRuntimeState& MacRuntimeState::GetInstance()
{
    return GetMutableRuntimeState();
}

MacRuntimeState::~MacRuntimeState()
{}

void InitializeProcessInfo()
{}

MacRuntimeState& GetMutableRuntimeState()
{
    static MacRuntimeState instance;
    return instance;
}

} // namespace Engine
