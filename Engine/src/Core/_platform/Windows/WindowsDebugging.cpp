#include <Engine/Core/Debugging.h>

#include <windows.h>

static_assert(ADHOC_WINDOWS);

namespace Engine::Platform
{

bool IsBeingDebuggedImpl()
{
    return ::IsDebuggerPresent() != FALSE;
}

} // namespace Engine::Platform
