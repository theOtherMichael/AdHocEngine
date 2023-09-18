#pragma once

#ifndef _WIN32
static_assert(false);
#endif // !_WIN32

#include <string>
#include <string_view>

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <Enterprise/Core/SharedLibraryExports.h>

ENTERPRISE_API std::string WCHARtoUTF8(const WCHAR* str, size_t length = 0);
ENTERPRISE_API std::wstring UTF8toWCHAR(const std::string_view str);

ENTERPRISE_API std::string GetLastErrorAsString();
