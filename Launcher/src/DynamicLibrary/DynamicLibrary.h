#pragma once

#include <functional>
#include <filesystem>
#include <string>
#include <unordered_map>

class DynamicLibrary
{
public:
    bool IsValid() { return libraryHandle != nullptr; }

    DynamicLibrary() = default;

    explicit DynamicLibrary(const std::filesystem::path& libraryPath) { Load(libraryPath); }

    DynamicLibrary(const DynamicLibrary& other) { Load(other.libraryPath); }

    DynamicLibrary& operator=(const DynamicLibrary& other)
    {
        if (this == &other)
            return *this;

        Unload();
        Load(other.libraryPath);
        return *this;
    }

    ~DynamicLibrary() { Unload(); }

    template <typename T>
    std::function<T> GetFunctionPtr(const std::string& functionName)
    {
        void* functionPtr = GetRawFunctionPtr(functionName);
        return std::function<T>(reinterpret_cast<T*>(functionPtr));
    }

private:
    std::filesystem::path libraryPath;
    void* libraryHandle = nullptr;

    void Load(const std::filesystem::path& path);
    void Unload();

    void* GetRawFunctionPtr(const std::string& functionName);
};
