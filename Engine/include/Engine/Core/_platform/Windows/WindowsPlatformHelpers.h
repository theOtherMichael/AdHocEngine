#pragma once

#include <Engine/Core/SymbolExportMacros.h>

#include <windows.h>

#include <string>
#include <string_view>

namespace Windows
{

ENGINE_API std::string WcharToUtf8(const WCHAR* str, size_t length = 0);
ENGINE_API std::wstring Utf8ToWchar(const std::string_view str);

ENGINE_API std::string GetLastErrorMessage();

} // namespace Windows
