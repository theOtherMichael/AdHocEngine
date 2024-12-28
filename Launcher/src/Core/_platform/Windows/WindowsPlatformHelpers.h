#pragma once

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <string>
#include <string_view>

#ifndef ADHOC_WINDOWS
static_assert(false);
#endif

namespace Windows
{

std::string WcharToUtf8(const WCHAR* wideString, size_t length = 0);
std::wstring Utf8ToWchar(const std::string_view narrowString);

std::string GetLastErrorMessage();

} // namespace Windows
