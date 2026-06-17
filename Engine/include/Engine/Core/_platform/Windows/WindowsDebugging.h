#pragma once

#if !ADHOC_RELEASE
/// Portably trigger a breakpoint in the debugger.
#define DEBUG_BREAK() (__nop(), __debugbreak())
#else
/// Portably trigger a breakpoint in the debugger.
#define DEBUG_BREAK()
#endif

namespace Engine::Platform
{

bool IsBeingDebuggedImpl();

} // namespace Engine::Platform
