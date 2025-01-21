#pragma once

#include <filesystem>
#include <functional>
#include <string>

class BaseDynamicLibrary
{
public:
    virtual bool IsValid() = 0;

    template <typename T>
    std::function<T> GetFunction(const std::string& functionName)
    {
        void* functionPtr = GetRawFunctionPtr(functionName);
        return std::function<T>(reinterpret_cast<T*>(functionPtr));
    }

protected:
    void* libraryHandle = nullptr;
    std::filesystem::path libraryPath;

    virtual void Load(const std::filesystem::path& path) = 0;
    virtual void Unload()                                = 0;

    virtual void* GetRawFunctionPtr(const std::string& functionName) = 0;
};
