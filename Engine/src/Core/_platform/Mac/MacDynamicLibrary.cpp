#include <Engine/Core/_platform/Mac/MacDynamicLibrary.h>

#include <Engine/Core/Assertions.h>

#include <dlfcn.h>

#include <iostream>

ASSERT_PLATFORM_MACOS;

namespace fs = std::filesystem;

void MacDynamicLibrary::Load(const std::filesystem::path& libraryPath)
{
    libraryHandle = dlopen(libraryPath.c_str(), RTLD_LAZY | RTLD_LOCAL | RTLD_FIRST);

    if (!libraryHandle)
        std::cerr << "Failed to load library " << libraryPath << "! " << dlerror() << "\n";
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
        std::cerr << "Attempted to load symbol " << functionName + " on an invalid DynamicLibrary!\n";
        return nullptr;
    }

    void* functionPtr = dlsym(libraryHandle, functionName.c_str());
    if (!functionPtr)
        std::cerr << "Failed to get symbol " << functionName + "! " << dlerror() << "\n";

    return functionPtr;
}
