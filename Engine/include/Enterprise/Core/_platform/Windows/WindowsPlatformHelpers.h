#pragma once

#include <Enterprise/Core/SymbolExportMacros.h>

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <string>
#include <string_view>

#ifndef ENTERPRISE_WINDOWS
static_assert(false);
#endif

namespace Windows
{

ENGINE_API std::string WcharToUtf8(const WCHAR* str, size_t length = 0);
ENGINE_API std::wstring Utf8ToWchar(const std::string_view str);

ENGINE_API std::string GetLastErrorMessage();

} // namespace Windows
