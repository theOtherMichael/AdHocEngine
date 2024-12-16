#pragma once

#ifndef ENTERPRISE_WINDOWS
static_assert(false);
#endif // !ENTERPRISE_WINDOWS

#include "SymbolExportMacros.h"

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Enterprise
{

struct ENGINE_API PlatformData_Win
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

    friend ENGINE_API PlatformData_Win* GetMutablePlatformData_Win();
};

#ifdef ENTERPRISE_INTERNAL
ENGINE_API PlatformData_Win* GetMutablePlatformData_Win();
#endif // ENTERPRISE_INTERNAL

} // namespace Enterprise
