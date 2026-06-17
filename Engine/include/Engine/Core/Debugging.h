#pragma once

#include <Engine/Core/PlatformAbstraction.h>
#include <Engine/Core/SymbolExportMacros.h>
#include PLATFORM_HEADER(Debugging.h)

#ifndef DEBUG_BREAK
static_assert(false, "DEBUG_BREAK() is not implemented for this platform");
#endif

namespace Engine
{

/// True when a debugger is attached to this process.
ENGINE_API bool IsBeingDebugged();

} // namespace Engine
