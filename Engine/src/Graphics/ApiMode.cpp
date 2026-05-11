#include <Engine/Graphics/ApiMode.h>

#include <Engine/Core/Assertions.h>
#include <Engine/Core/Console.h>
#include <Engine/Graphics/GraphicsContext.h>

#include <algorithm>
#include <vector>

namespace Engine::Graphics
{

static auto currentApiMode = ApiMode::Uninitialized;

std::vector<ApiMode> GetSupportedApiModes()
{
    // TODO: Replace with an actual hardware check

#if ADHOC_WINDOWS
    return std::vector{ApiMode::D3d11};
#elif ADHOC_MAC
    return std::vector{ApiMode::Metal};
#else
    Assert_NoEntry();
#endif
}

ApiMode GetDefaultApiMode()
{
    // TODO: Replace with an actual hardware check

#if ADHOC_WINDOWS
    return ApiMode::D3d11;
#elif ADHOC_MAC
    return ApiMode::Metal;
#else
    Assert_NoEntry();
#endif
}

ApiMode GetActiveApiMode()
{
    return currentApiMode;
}

void SetApiMode(ApiMode apiMode)
{
    const auto compatibleApiModes = GetSupportedApiModes();
    const auto isApiModeSupported = std::ranges::contains(compatibleApiModes, apiMode);

    if (isApiModeSupported)
    {
        Internal::CreateContext(apiMode);
        currentApiMode = apiMode;
    }
    else
    {
        Console::LogError("{} is not supported on this device", apiMode);
    }
}

} // namespace Engine::Graphics
