#include <Engine/File/SystemPaths.h>

#include <Engine/Core/PlatformAbstraction.h>
#include PLATFORM_HEADER(SystemPaths.h)

#include <filesystem>

namespace Engine
{

namespace fs = std::filesystem;

fs::path GetExecutablePath()
{
    return Platform::GetExecutablePathImpl();
}

} // namespace Engine
