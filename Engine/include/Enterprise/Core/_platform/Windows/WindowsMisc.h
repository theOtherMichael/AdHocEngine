#pragma once

#include <Enterprise/Core/SymbolExportMacros.h>

#include <malloc.h>

namespace Engine
{

#ifndef ENTERPRISE_RELEASE
    /// Portably trigger a breakpoint in the debugger.
    #define DEBUGBREAK() (__nop(), __debugbreak())
#else
    /// Portably trigger a breakpoint in the debugger.
    #define DEBUGBREAK()
#endif

/// Portably allocate on the extended stack.
void* StackAlloc(size_t size)
{
    return _malloca(size);
}

/// Print the current call stack.
void ENGINE_API PrintBacktrace();

} // namespace Engine
