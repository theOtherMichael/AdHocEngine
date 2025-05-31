#include <Engine/Core/_platform/Windows/WindowsPlatformHelpers.h>

#include <Engine/Core/Assertions.h>

#include <windows.h>

ASSERT_PLATFORM_WINDOWS;

namespace Windows
{

std::string WcharToUtf8(const WCHAR* wideString, size_t length)
{
    if (length == 0)
        length = wcslen(wideString);

    if (length == 0)
        return std::string();

    std::string convertedString(WideCharToMultiByte(CP_UTF8, 0, wideString, (int)length, NULL, 0, NULL, NULL), 0);

    WideCharToMultiByte(
        CP_UTF8, 0, wideString, (int)length, &convertedString[0], (int)convertedString.size(), NULL, NULL);

    return convertedString;
}

std::wstring Utf8ToWchar(const std::string_view narrowString)
{
    if (narrowString.length() == 0)
        return std::wstring();

    std::wstring convertedString(MultiByteToWideChar(CP_UTF8, 0, narrowString.data(), -1, NULL, 0), 0);

    MultiByteToWideChar(CP_UTF8, 0, narrowString.data(), -1, convertedString.data(), (int)convertedString.size());

    return convertedString;
}

std::string GetLastErrorMessage()
{
    DWORD errorCode = GetLastError();

    LPVOID messageBuffer = nullptr;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  errorCode,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR)&messageBuffer,
                  0,
                  NULL);

    std::string formattedMessage = std::to_string(errorCode) + ", " + WcharToUtf8((LPTSTR)messageBuffer);

    LocalFree(messageBuffer);

    return formattedMessage;
}

} // namespace Windows
