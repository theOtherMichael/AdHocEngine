#include <Engine/Common/_platform/Windows/WindowsPlatformHelpers.h>

#include <asl/casts.h>
#include <asl/finally.h>
#include <Engine/Core/Assertions.h>

#include <fmt/format.h>

#include <windows.h>

static_assert(ADHOC_WINDOWS);

namespace Engine::Platform
{

std::string WcharToUtf8(const WCHAR* wideString, size_t length)
{
    if (length == 0)
        length = wcslen(wideString);

    if (length == 0)
        return std::string{};

    const auto convertedStringSize =
        WideCharToMultiByte(CP_UTF8, 0, wideString, asl::narrow<int>(length), NULL, 0, NULL, NULL);

    auto convertedString = std::string(convertedStringSize, 0);

    WideCharToMultiByte(
        CP_UTF8, 0, wideString, asl::narrow<int>(length), convertedString.data(), convertedStringSize, NULL, NULL);

    return convertedString;
}

std::wstring Utf8ToWchar(const std::string_view narrowString)
{
    if (narrowString.length() == 0)
        return std::wstring{};

    const auto convertedStringSize =
        MultiByteToWideChar(CP_UTF8, 0, narrowString.data(), narrowString.length(), NULL, 0);

    auto convertedString = std::wstring(convertedStringSize, 0);

    MultiByteToWideChar(
        CP_UTF8, 0, narrowString.data(), narrowString.length(), convertedString.data(), convertedStringSize);

    return convertedString;
}

std::string GetLastErrorMessage()
{
    const auto errorCode = GetLastError();

    auto messageBuffer = LPTSTR{nullptr};

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  errorCode,
                  0,
                  reinterpret_cast<LPTSTR>(&messageBuffer), // Looks weird, but it's correct
                  0,
                  NULL);

    const auto freeMessageBufferAction = asl::finally([messageBuffer]() { LocalFree(messageBuffer); });

    return fmt::format("{}, {}", std::to_string(errorCode), WcharToUtf8(messageBuffer));
}

} // namespace Engine::Platform
