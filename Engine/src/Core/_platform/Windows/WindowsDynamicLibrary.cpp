#include "WindowsDynamicLibrary.h"

#include <Engine/Common/PlatformHelpers.h>
#include <Engine/Core/Assertions.h>
#include <Engine/Core/Console.h>

#include <windows.h>

#include <filesystem>
#include <type_traits>

static_assert(ADHOC_WINDOWS);

namespace fs = std::filesystem;

namespace Engine::Platform
{

void DynamicLibraryImplementation::Load(const fs::path& libraryPath)
{
    static_assert(std::is_same_v<fs::path::value_type, wchar_t>);

    libraryHandle = LoadLibraryEx(libraryPath.wstring().c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

    if (!IsValidImpl())
        Console::LogError("Library \"{}\" could not be loaded. {}", libraryPath.string(), GetLastErrorMessage());
}

void DynamicLibraryImplementation::Unload()
{
    if (IsValidImpl())
    {
        FreeLibrary(libraryHandle);
        libraryHandle = NULL;
    }
}

void* DynamicLibraryImplementation::GetRawFunctionPtr(const std::string& functionName) const
{
    if (!IsValidImpl())
    {
        Console::LogError("Symbol \"{}\" cannot be loaded; the library handle is invalid.", functionName);
        return nullptr;
    }

    auto functionPtr = GetProcAddress(libraryHandle, functionName.c_str());
    if (functionPtr == NULL)
        Console::LogError("Symbol \"{}\" could not be retrieved. {}", functionName, GetLastErrorMessage());

    return reinterpret_cast<void*>(functionPtr);
}

} // namespace Engine::Platform
