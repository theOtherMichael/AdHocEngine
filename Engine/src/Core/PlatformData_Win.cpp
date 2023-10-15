#include <Enterprise/Core/PlatformData_Win.h>

namespace Enterprise
{

PlatformData_Win* PlatformData_Win::instance = nullptr;

const PlatformData_Win* PlatformData_Win::Get()
{
    return GetMutablePlatformData_Win();
}

PlatformData_Win* GetMutablePlatformData_Win()
{
    if (!PlatformData_Win::instance)
        PlatformData_Win::instance = new PlatformData_Win();

    return PlatformData_Win::instance;
}

} // namespace Enterprise
