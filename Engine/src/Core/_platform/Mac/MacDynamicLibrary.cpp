#include <Engine/Core/_platform/Mac/MacDynamicLibrary.h>

#include <Engine/Core/Assertions.h>
#include <Engine/Core/Console.h>

#include <dlfcn.h>

static_assert(ADHOC_MAC);

namespace Console = Engine::Console;
namespace fs      = std::filesystem;

void MacDynamicLibrary::Load(const std::filesystem::path& libraryPath)
{
    libraryHandle = dlopen(libraryPath.c_str(), RTLD_LAZY | RTLD_LOCAL | RTLD_FIRST);

    if (!libraryHandle)
        Console::LogError("Library \"{}\" could not be loaded. {}", libraryPath.string(), dlerror());
    else
        this->libraryPath = libraryPath;
}

void MacDynamicLibrary::Unload()
{
    if (libraryHandle)
        dlclose(libraryHandle);

    libraryPath.clear();
    libraryHandle = nullptr;
}

void* MacDynamicLibrary::GetRawFunctionPtr(const std::string& functionName)
{
    if (!libraryHandle)
    {
        Console::LogError("Symbol \"{}\" cannot be loaded; the library handle is invalid.", functionName);
        return nullptr;
    }

    void* functionPtr = dlsym(libraryHandle, functionName.c_str());
    if (!functionPtr)
        Console::LogError("Symbol \"{}\" could not be retrieved. {}", functionName, dlerror());

    return functionPtr;
}
