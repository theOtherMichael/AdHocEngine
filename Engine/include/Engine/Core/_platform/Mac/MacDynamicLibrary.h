#pragma once

#include "../Base/BaseDynamicLibrary.h"

class MacDynamicLibrary : public BaseDynamicLibrary
{
public:
    bool IsValid() override final { return libraryHandle != nullptr; }

    MacDynamicLibrary() = default;

    explicit MacDynamicLibrary(const std::filesystem::path& libraryPath) { Load(libraryPath); }

    MacDynamicLibrary(const MacDynamicLibrary& other) { Load(other.libraryPath); }

    MacDynamicLibrary& operator=(const MacDynamicLibrary& other)
    {
        if (this == &other)
            return *this;

        Unload();
        Load(other.libraryPath);
        return *this;
    }

private:
    void* libraryHandle = nullptr;

    void Load(const std::filesystem::path& libraryPath) override final;
    void Unload() override final;
    void* GetRawFunctionPtr(const std::string& functionName) override final;
};

typedef MacDynamicLibrary DynamicLibrary;
