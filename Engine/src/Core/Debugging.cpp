#include <Engine/Core/Debugging.h>

#include <string>

namespace Engine
{

bool IsBeingDebugged()
{
    return Platform::IsBeingDebuggedImpl();
}

std::string GetBacktrace()
{
    return Platform::GetBacktraceImpl();
}

} // namespace Engine
