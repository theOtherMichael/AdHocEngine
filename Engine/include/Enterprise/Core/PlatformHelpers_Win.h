#pragma once

#ifndef ENTERPRISE_WINDOWS
static_assert(false);
#endif // !ENTERPRISE_WINDOWS

#include <string>
#include <string_view>

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <Enterprise/Core/SymbolExportMacros.h>

ENGINE_API std::string WCHARtoUTF8(const WCHAR* str, size_t length = 0);
ENGINE_API std::wstring UTF8toWCHAR(const std::string_view str);

ENGINE_API std::string GetLastErrorAsString();
