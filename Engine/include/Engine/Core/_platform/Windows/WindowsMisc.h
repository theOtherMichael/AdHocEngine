#pragma once

#include <Engine/Core/SymbolExportMacros.h>

#include <malloc.h>

#include <string>

namespace Engine
{

#if !ADHOC_RELEASE
    /// Portably trigger a breakpoint in the debugger.
    #define DEBUG_BREAK() (__nop(), __debugbreak())
#else
    /// Portably trigger a breakpoint in the debugger.
    #define DEBUG_BREAK()
#endif

/// Portably allocate on the extended stack.
inline void* StackAlloc(size_t size)
{
    return _malloca(size);
}

/// Print the current call stack.
ENGINE_API std::string GetBacktrace();

} // namespace Engine
