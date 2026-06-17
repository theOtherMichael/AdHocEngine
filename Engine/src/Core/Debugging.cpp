#include <Engine/Core/Debugging.h>

namespace Engine
{

bool IsBeingDebugged()
{
    return Platform::IsBeingDebuggedImpl();
}

} // namespace Engine
