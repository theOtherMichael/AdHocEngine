#pragma once

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

#include <string>

namespace Engine::Platform
{

bool IsBeingDebuggedImpl();

std::string GetBacktraceImpl();

} // namespace Engine::Platform
