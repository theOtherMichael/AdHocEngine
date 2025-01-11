#pragma once

#include <alloca.h>

#include <string>

namespace Engine
{

#ifndef ADHOC_RELEASE
    /// Portably trigger a breakpoint in the debugger.
    #define DEBUG_BREAK() __asm__("int $3")
#else
    /// Portably trigger a breakpoint in the debugger.
    #define DEBUG_BREAK()
#endif

/// Portably allocate on the extended stack.
inline void* StackAlloc(size_t size)
{
    return alloca(size);
}

/// Print the current call stack.
std::string GetBacktrace();

} // namespace Engine
