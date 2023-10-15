#pragma once

#include "SharedLibraryExports.h"

#ifndef ENTERPRISE_RELEASE
    #ifdef _WIN32
        /// Portably trigger a breakpoint in the debugger.
        #define DEBUGBREAK() (__nop(), __debugbreak())
    #else
        /// Portably trigger a breakpoint in the debugger.
        #define DEBUGBREAK() __asm__("int $3")
    #endif // _WIN32
#else
    /// Portably trigger a breakpoint in the debugger.
    #define DEBUGBREAK()
#endif // !ENTERPRISE_RELEASE

/// Rapidly assemble bit fields.
#define BIT(x) (1ull << (x))

#ifdef _WIN32
    /// Portably allocate on the extended stack.
    #define STACKALLOC(size) _malloca(size)
#else
    /// Portably allocate on the extended stack.
    #define STACKALLOC(size) alloca(size)
#endif // _WIN32

#define STRINGIFY_IMPL(macro) #macro
#define STRINGIFY(macro) STRINGIFY_IMPL(macro)

namespace Enterprise
{

void ENTERPRISE_API PrintBacktrace();

} // namespace Enterprise
