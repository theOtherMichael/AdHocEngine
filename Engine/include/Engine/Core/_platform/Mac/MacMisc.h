#pragma once

#include "../Base/BaseMisc.h"

#include <alloca.h>

#include <string>

namespace Engine
{

#if !ADHOC_RELEASE
    #ifdef __arm64__
        /// Portably trigger a breakpoint in the debugger.
        #define DEBUG_BREAK() __builtin_debugtrap()
    #else
        /// Portably trigger a breakpoint in the debugger.
        #define DEBUG_BREAK() __asm__("int $3")
    #endif
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
