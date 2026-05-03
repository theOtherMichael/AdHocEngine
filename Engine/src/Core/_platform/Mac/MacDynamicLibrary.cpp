#include "MacDynamicLibrary.h"

#include <Engine/Core/Assertions.h>
#include <Engine/Core/Console.h>

#include <dlfcn.h>

static_assert(ADHOC_MAC);

namespace fs = std::filesystem;

namespace Engine::Platform
{

void DynamicLibraryImplementation::Load(const fs::path& libraryPath)
{
    libraryHandle = dlopen(libraryPath.c_str(), RTLD_LAZY | RTLD_LOCAL | RTLD_FIRST);

    if (!IsValidImpl())
        Console::LogError("Library \"{}\" could not be loaded. {}", libraryPath.string(), dlerror());
}

void DynamicLibraryImplementation::Unload()
{
    if (IsValidImpl())
    {
        dlclose(libraryHandle);
        libraryHandle = nullptr;
    }
}

void* DynamicLibraryImplementation::GetRawFunctionPtr(const std::string& functionName) const
{
    if (!IsValidImpl())
    {
        Console::LogError("Symbol \"{}\" cannot be loaded; the library handle is invalid.", functionName);
        return nullptr;
    }

    void* functionPtr = dlsym(libraryHandle, functionName.c_str());
    if (!functionPtr)
        Console::LogError("Symbol \"{}\" could not be retrieved. {}", functionName, dlerror());

    return functionPtr;
}

} // namespace Engine::Platform
