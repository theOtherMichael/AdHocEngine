#pragma once

#include "SymbolExportMacros.h"

#ifndef ENTERPRISE_RELEASE
    #ifdef ENTERPRISE_WINDOWS
        /// Portably trigger a breakpoint in the debugger.
        #define DEBUGBREAK() (__nop(), __debugbreak())
    #else
        /// Portably trigger a breakpoint in the debugger.
        #define DEBUGBREAK() __asm__("int $3")
    #endif // ENTERPRISE_WINDOWS
#else
    /// Portably trigger a breakpoint in the debugger.
    #define DEBUGBREAK()
#endif // !ENTERPRISE_RELEASE

/// Rapidly assemble bit fields.
#define BIT(x) (1ull << (x))

#ifdef ENTERPRISE_WINDOWS
    /// Portably allocate on the extended stack.
    #define STACKALLOC(size) _malloca(size)
#else
    /// Portably allocate on the extended stack.
    #define STACKALLOC(size) alloca(size)
#endif // ENTERPRISE_WINDOWS

#define STRINGIFY_IMPL(macro) #macro
#define STRINGIFY(macro) STRINGIFY_IMPL(macro)

namespace Enterprise
{

void ENGINE_API PrintBacktrace();

} // namespace Enterprise
