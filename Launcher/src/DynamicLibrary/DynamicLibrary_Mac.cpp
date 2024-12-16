#include "DynamicLibrary.h"

#if !(defined(__APPLE__) && defined(__MACH__))
static_assert(false);
#endif

#include <iostream>

#include <dlfcn.h>

namespace fs = std::filesystem;

void DynamicLibrary::Load(const fs::path& libraryPath)
{
    libraryHandle = dlopen(libraryPath.c_str(), RTLD_LAZY | RTLD_LOCAL | RTLD_FIRST);

    if (!libraryHandle)
        std::cerr << "Failed to load library " << libraryPath << "! " << dlerror() << "\n";
    else
        this->libraryPath = libraryPath;
}

void DynamicLibrary::Unload()
{
    if (libraryHandle)
        dlclose(libraryHandle);

    libraryPath.clear();
    libraryHandle = nullptr;
}

void* DynamicLibrary::GetRawFunctionPtr(const std::string& functionName)
{
    if (!libraryHandle)
    {
        std::cerr << "Attempted to load symbol " << functionName + " on an invalid DynamicLibrary!\n";
        return;
    }

    void* functionPtr = dlsym(libraryHandle, functionName.c_str());
    if (!functionPtr)
        std::cerr << "Failed to get symbol " << functionName + "! " << dlerror() << "\n";

    return functionPtr;
}
