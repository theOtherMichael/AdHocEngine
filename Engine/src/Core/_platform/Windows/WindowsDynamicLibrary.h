#pragma once

#include <windows.h>

#include <filesystem>

namespace Engine::Platform
{

class DynamicLibraryImplementation
{
public:
    bool IsValidImpl() const { return libraryHandle != NULL; }

    void Load(const std::filesystem::path& libraryPath);
    void Unload();

    void* GetRawFunctionPtr(const std::string& functionName) const;

private:
    HMODULE libraryHandle = NULL;
};

} // namespace Engine::Platform
