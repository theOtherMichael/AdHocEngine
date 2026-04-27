#pragma once

#include <Engine/Core/SymbolExportMacros.h>

#include <windows.h>

namespace Engine::Platform
{

class ENGINE_API PlatformRuntimeInfo
{
public:
    PlatformRuntimeInfo();
    ~PlatformRuntimeInfo();

    HANDLE processHandle = NULL;
};

} // namespace Engine::Platform
