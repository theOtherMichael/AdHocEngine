#pragma once

#include <Engine/Core/SymbolExportMacros.h>

#include <windows.h>

namespace Engine::Platform
{

class PlatformRuntimeInfo
{
public:
    ENGINE_API PlatformRuntimeInfo();
    ENGINE_API ~PlatformRuntimeInfo();

    HANDLE processHandle = NULL;
};

} // namespace Engine::Platform
