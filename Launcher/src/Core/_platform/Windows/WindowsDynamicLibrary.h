#pragma once

#include "../Base/BaseDynamicLibrary.h"

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#ifndef ENTERPRISE_WINDOWS
static_assert(false);
#endif

class WindowsDynamicLibrary : public BaseDynamicLibrary
{
public:
    bool IsValid() override final { return libraryHandle != NULL; }

    WindowsDynamicLibrary() = default;

    explicit WindowsDynamicLibrary(const std::filesystem::path& libraryPath) { Load(libraryPath); }

    WindowsDynamicLibrary(const WindowsDynamicLibrary& other) { Load(other.libraryPath); }

    WindowsDynamicLibrary& operator=(const WindowsDynamicLibrary& other)
    {
        if (this == &other)
            return *this;

        Unload();
        Load(other.libraryPath);
        return *this;
    }

private:
    HMODULE libraryHandle = NULL;

    void Load(const std::filesystem::path& libraryPath) override final;
    void Unload() override final;
    void* GetRawFunctionPtr(const std::string& functionName) override final;
};

typedef WindowsDynamicLibrary DynamicLibrary;
