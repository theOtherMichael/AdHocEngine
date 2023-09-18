#include <Enterprise/Core/PlatformHelpers_Win.h>

#include <string>

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <windows.h>

std::string WCHARtoUTF8(const WCHAR* wideString, size_t length)
{
    if (length == 0)
        length = wcslen(wideString);

    if (length > 0)
    {
        std::string convertedString(
            WideCharToMultiByte(CP_UTF8, 0, wideString, (int)length, NULL, 0, NULL, NULL),
            0);

        WideCharToMultiByte(CP_UTF8,
                            0,
                            wideString,
                            (int)length,
                            &convertedString[0],
                            (int)convertedString.size(),
                            NULL,
                            NULL);

        return convertedString;
    }
    else
    {
        return std::string();
    }
}

std::wstring UTF8toWCHAR(const std::string_view narrowString)
{
    if (narrowString.length() > 0)
    {
        std::wstring convertedString(
            MultiByteToWideChar(CP_UTF8, 0, narrowString.data(), -1, NULL, 0), 0);

        MultiByteToWideChar(CP_UTF8,
                            0,
                            narrowString.data(),
                            -1,
                            convertedString.data(),
                            (int)convertedString.size());

        return convertedString;
    }
    else
    {
        return std::wstring();
    }
}

std::string GetLastErrorAsString()
{
    DWORD errorCode = GetLastError();

    LPVOID messageBuffer = nullptr;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                      FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  errorCode,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR)&messageBuffer,
                  0,
                  NULL);

    std::string formattedMessage =
        std::to_string(errorCode) + ", " + WCHARtoUTF8((LPTSTR)messageBuffer);

    LocalFree(messageBuffer);

    return formattedMessage;
}
