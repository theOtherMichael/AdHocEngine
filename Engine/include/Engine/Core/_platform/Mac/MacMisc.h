#pragma once

#include <alloca.h>

namespace Engine
{

#ifndef ADHOC_RELEASE
    /// Portably trigger a breakpoint in the debugger.
    #define DEBUGBREAK() __asm__("int $3")
#else
    /// Portably trigger a breakpoint in the debugger.
    #define DEBUGBREAK()
#endif

/// Portably allocate on the extended stack.
inline void* StackAlloc(size_t size)
{
    return alloca(size);
}

/// Print the current call stack.
void PrintBacktrace();

} // namespace Engine
