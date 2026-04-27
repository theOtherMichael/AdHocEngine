#include "WindowsMisc.h"

#include <Engine/Common/PlatformHelpers.h>
#include <Engine/Core/Assertions.h>
#include <Engine/Core/Console.h>
#include <Engine/Core/RuntimeInfo.h>

#include <fmt/format.h>

#include <DbgHelp.h>
#include <windows.h>

#include <array>
#include <filesystem>
#include <sstream>
#include <string>

#include <malloc.h>

static_assert(ADHOC_WINDOWS);

namespace fs = std::filesystem;

namespace Engine::Platform
{

void* StackAllocImpl(size_t size)
{
    return _malloca(size);
}

template <size_t maxNameLength = 256>
class SymbolInfo
{
public:
    SymbolInfo() noexcept : rawSymbolInfo(new (buffer) SYMBOL_INFO{})
    {
        static_assert(maxNameLength > 0);

        rawSymbolInfo->SizeOfStruct = sizeof(SYMBOL_INFO);
        rawSymbolInfo->MaxNameLen   = static_cast<ULONG>(maxNameLength);
    }

    SymbolInfo(const SymbolInfo&)            = delete;
    SymbolInfo& operator=(const SymbolInfo&) = delete;

    SymbolInfo(SymbolInfo&&)            = delete;
    SymbolInfo& operator=(SymbolInfo&&) = delete;

    [[nodiscard]] SYMBOL_INFO* data() { return rawSymbolInfo; }

    [[nodiscard]] const SYMBOL_INFO* data() const { return rawSymbolInfo; }

    [[nodiscard]] SYMBOL_INFO* operator->() { return rawSymbolInfo; }

    [[nodiscard]] const SYMBOL_INFO* operator->() const { return rawSymbolInfo; }

private:
    alignas(SYMBOL_INFO) std::byte buffer[sizeof(SYMBOL_INFO) + maxNameLength - 1];

    SYMBOL_INFO* const rawSymbolInfo;
};

std::string GetBacktraceImpl()
{
    // TODO: Make maxFrames configurable in editor
    constexpr auto maxFrames = 64;
    auto callStack           = std::array<void*, maxFrames>{};

    auto frames        = CaptureStackBackTrace(0, maxFrames, callStack.data(), NULL);
    auto currentSymbol = SymbolInfo<256>{};

    const auto& runtimeInfo  = RuntimeInfo::Instance();
    const auto processHandle = runtimeInfo.platformInfo->processHandle;

    if (!SymInitialize(processHandle, NULL, TRUE))
    {
        auto errorMessage = GetLastErrorMessage();
        Console::LogError("Symbol initialization failed; backtraces are unavailable this session. {}", errorMessage);
        return std::string{};
    }

    auto backtraceStringStream = std::ostringstream{};
    for (int i = 0; i < frames; ++i)
    {
        if (!SymFromAddr(processHandle, reinterpret_cast<DWORD64>(callStack[i]), 0, currentSymbol.data()))
        {
            backtraceStringStream << fmt::format("[{}] Symbol address lookup failed. {}\n", i, GetLastErrorMessage());
            continue;
        }

        backtraceStringStream << fmt::format("[{}] (0x{}) {}\n", i, currentSymbol->Address, currentSymbol->Name);
    }

    if (!SymCleanup(runtimeInfo.platformInfo->processHandle))
        Console::LogError("Symbol cleanup failed. {}", GetLastErrorMessage());

    return backtraceStringStream.str();
}

fs::path GetExecutablePathImpl()
{
    auto rawPath = std::array<TCHAR, MAX_PATH>{};
    if (!GetModuleFileName(NULL, rawPath.data(), MAX_PATH))
    {
        Console::LogError("Executable path is unavailable. Error: {}", GetLastErrorMessage());
        return fs::path{};
    }
    return fs::path{rawPath.data()};
}

} // namespace Engine::Platform
