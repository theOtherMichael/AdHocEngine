#include "DynamicLibrary.h"

#ifndef _WIN32
static_assert(false);
#endif

#include <iostream>
#include <filesystem>

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "../Misc/WindowsHelpers.h"

namespace fs = std::filesystem;

void DynamicLibrary::Load(const fs::path& libraryPath)
{
    libraryHandle = LoadLibraryEx(libraryPath.wstring().c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

    if (!libraryHandle)
        std::cerr << "Failed to load library " << libraryPath << "! " << Misc::Win::GetLastErrorMessage() << "\n";
    else
        this->libraryPath = libraryPath;
}

void DynamicLibrary::Unload()
{
    if (libraryHandle)
        FreeLibrary(static_cast<HMODULE>(libraryHandle));

    libraryPath.clear();
    libraryHandle = nullptr;
}

void* DynamicLibrary::GetRawFunctionPtr(const std::string& functionName)
{
    if (!libraryHandle)
    {
        std::cerr << "Attempted to load symbol " << functionName + " on an invalid DynamicLibrary!\n";
        return nullptr;
    }

    void* functionPtr = GetProcAddress(static_cast<HMODULE>(libraryHandle), functionName.c_str());
    if (!functionPtr)
        std::cerr << "Failed to get symbol " << functionName + "! " << Misc::Win::GetLastErrorMessage() << "\n";

    return functionPtr;
}
