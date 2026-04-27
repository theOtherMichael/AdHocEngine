#include <Engine/Core/Misc.h>

#include <Engine/Core/PlatformAbstraction.h>
#include PLATFORM_HEADER(Misc.h)

#include <filesystem>

namespace Engine
{

namespace fs = std::filesystem;

void* StackAlloc(size_t size)
{
    return Platform::StackAllocImpl(size);
}

std::string GetBacktrace()
{
    return Platform::GetBacktraceImpl();
}

fs::path GetExecutablePath()
{
    return Platform::GetExecutablePathImpl();
}

} // namespace Engine
