#pragma once

#include <filesystem>
#include <string>

namespace Engine::Platform
{

class DynamicLibraryImplementation
{
public:
    bool IsValidImpl() const { return libraryHandle != nullptr; }

    void Load(const std::filesystem::path& libraryPath);
    void Unload();

    void* GetRawFunctionPtr(const std::string& functionName) const;

private:
    void* libraryHandle = nullptr;
};

} // namespace Engine::Platform
