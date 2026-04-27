#pragma once

#include <concepts>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>

namespace Engine
{

class DynamicLibrary
{
public:
    DynamicLibrary();
    ~DynamicLibrary();

    DynamicLibrary(const std::filesystem::path& path);

    DynamicLibrary(const DynamicLibrary&)            = delete;
    DynamicLibrary& operator=(const DynamicLibrary&) = delete;

    DynamicLibrary(DynamicLibrary&&)            = delete;
    DynamicLibrary& operator=(DynamicLibrary&&) = delete;

    bool IsValid() const;

    void Load(const std::filesystem::path& path);
    void Unload();

    template <std::invocable T>
    std::function<T> GetFunction(const std::string& functionName) const
    {
        void* functionPtr = GetRawFunctionPtr(functionName);
        return std::function<T>(reinterpret_cast<T*>(functionPtr));
    }

private:
    struct Implementation;
    const std::unique_ptr<Implementation> implementation;

    void* GetRawFunctionPtr(const std::string& functionName) const;
};

} // namespace Engine
