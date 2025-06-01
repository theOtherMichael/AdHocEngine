#include <Engine/Core/_platform/Mac/MacPlatformData.h>

#include <Engine/Core/Assertions.h>

ASSERT_PLATFORM_MACOS;

namespace Engine
{

const MacPlatformData& MacPlatformData::GetInstance()
{
    return GetMutablePlatformData();
}

MacPlatformData::~MacPlatformData() {}

void InitializeProcessInfo() {}

MacPlatformData& GetMutablePlatformData()
{
    static MacPlatformData instance;
    return instance;
}

} // namespace Engine
