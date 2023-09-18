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

std::string WCHARtoUTF8(const WCHAR* wideString, size_t length = 0);
std::wstring UTF8toWCHAR(const std::string_view narrowString);

std::string GetLastErrorAsString();
