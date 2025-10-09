#pragma once

#include <Engine/Core/SymbolExportMacros.h>

#include <windows.h>

namespace Engine
{

struct ENGINE_API WindowsRuntimeState
{
public:
    HANDLE processHandle = NULL;

    static const WindowsRuntimeState& GetInstance();

    WindowsRuntimeState() = default;
    ~WindowsRuntimeState();

    WindowsRuntimeState(const WindowsRuntimeState&)            = delete;
    WindowsRuntimeState& operator=(const WindowsRuntimeState&) = delete;
};

#if ADHOC_INTERNAL
ENGINE_API void InitializeProcessInfo();
ENGINE_API WindowsRuntimeState& GetMutableRuntimeState();
#endif

typedef WindowsRuntimeState RuntimeState;

} // namespace Engine
