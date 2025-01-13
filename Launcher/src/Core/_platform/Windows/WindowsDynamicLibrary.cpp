#include "WindowsDynamicLibrary.h"

#include "Core/PlatformHelpers.h"

#include <windows.h>

#include <filesystem>
#include <iostream>

#if !ADHOC_WINDOWS
static_assert(false);
#endif

namespace fs = std::filesystem;

void WindowsDynamicLibrary::Load(const fs::path& libraryPath)
{
    libraryHandle = LoadLibraryEx(libraryPath.wstring().c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

    if (libraryHandle == NULL)
        std::cerr << "Failed to load library " << libraryPath << "! " << Windows::GetLastErrorMessage() << "\n";
    else
        this->libraryPath = libraryPath;
}

void WindowsDynamicLibrary::Unload()
{
    if (libraryHandle != NULL)
        FreeLibrary(libraryHandle);

    libraryPath.clear();
    libraryHandle = NULL;
}

void* WindowsDynamicLibrary::GetRawFunctionPtr(const std::string& functionName)
{
    if (libraryHandle == NULL)
    {
        std::cerr << "Attempted to load symbol " << functionName + " on an invalid DynamicLibrary!\n";
        return nullptr;
    }

    void* functionPtr = GetProcAddress(libraryHandle, functionName.c_str());
    if (!functionPtr)
        std::cerr << "Failed to get symbol " << functionName + "! " << Windows::GetLastErrorMessage() << "\n";

    return functionPtr;
}
