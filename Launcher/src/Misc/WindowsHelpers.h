#pragma once

#include <string>
#include <string_view>

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Misc::Win
{

std::string GetLastErrorMessage();

std::string ToUtf8(const WCHAR* wideString, size_t length = 0);

std::wstring ToWchar(const std::string_view narrowString);

} // namespace Misc::Win
