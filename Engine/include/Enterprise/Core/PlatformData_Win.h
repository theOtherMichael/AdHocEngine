#pragma once

#ifndef _WIN32
static_assert(false);
#endif // !_WIN32

#include "SharedLibraryExports.h"

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Enterprise
{

struct ENTERPRISE_API PlatformData_Win
{
public:
    static const PlatformData_Win* Get();

    HANDLE processHandle = NULL;

private:
    static PlatformData_Win* instance;

    PlatformData_Win()                                         = default;
    ~PlatformData_Win()                                        = default;
    PlatformData_Win(const PlatformData_Win& other)            = delete;
    PlatformData_Win(PlatformData_Win&& other)                 = delete;
    PlatformData_Win& operator=(const PlatformData_Win& other) = delete;

    friend ENTERPRISE_API PlatformData_Win* GetMutablePlatformData_Win();
};

#ifdef ENTERPRISE_INTERNAL
ENTERPRISE_API PlatformData_Win* GetMutablePlatformData_Win();
#endif // ENTERPRISE_INTERNAL

} // namespace Enterprise
