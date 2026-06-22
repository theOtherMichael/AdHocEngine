#pragma once

#if !ADHOC_RELEASE
/// Portably trigger a breakpoint in the debugger.
#define DEBUG_BREAK() (__nop(), __debugbreak())
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
