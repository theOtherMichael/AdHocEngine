#include <Engine/Graphics/GraphicsContext.h>

#include <Engine/Common/ThreadSafeViews.h>
#include <Engine/Core/Assertions.h>
#include <Engine/Graphics/_platform/Base/BaseGraphicsContext.h>
#include <Engine/Graphics/D3D11GraphicsContext.h>

#include <memory>
#include <shared_mutex>

namespace Engine::Graphics
{

static auto currentApiMode              = ApiMode::None;
static auto currentGraphicsContext      = std::unique_ptr<BaseGraphicsContext>{};
static auto currentGraphicsContextMutex = std::shared_mutex{};

bool SetApiMode(ApiMode apiMode)
{
    switch (apiMode)
    {
    case Engine::Graphics::ApiMode::None:
        Console::LogError("Setting API mode to \"None\" is not allowed!");
        return false;
    case Engine::Graphics::ApiMode::OpenGl: Console::LogError("Not yet implemented!"); return false;
    case Engine::Graphics::ApiMode::Vulkan: Console::LogError("Not yet implemented!"); return false;
    case Engine::Graphics::ApiMode::D3D11:
    {
#if ADHOC_WINDOWS
        auto lock              = std::unique_lock(currentGraphicsContextMutex);
        currentGraphicsContext = std::make_unique<D3D11GraphicsContext>();
        break;
#else
        Console::LogError("D3D11 not supported on this platform!");
        return false;
#endif
    }
    case Engine::Graphics::ApiMode::D3D12:
#if ADHOC_WINDOWS
        Console::LogError("Not yet implemented!");
        return false;
#else
        Console::LogError("D3D12 is not supported on this platform!");
        return false;
#endif

    case Engine::Graphics::ApiMode::Metal: Console::LogError("Not yet implemented!"); return false;
    default: Assert_NoEntry(); break;
    }

    currentApiMode = apiMode;
    return true;
}

ApiMode GetApiMode()
{
    return currentApiMode;
}

BaseGraphicsContext& Internal::GetContextUnsafe()
{
    Assert_NotNull(currentGraphicsContext.get());
    return *currentGraphicsContext.get();
}

std::shared_mutex& Internal::GetContextMutex()
{
    return currentGraphicsContextMutex;
}

void Internal::ShutdownContext()
{
    auto lock = std::unique_lock(currentGraphicsContextMutex);
    currentGraphicsContext.reset(nullptr);
}

ThreadSafeSharedView<BaseGraphicsContext> GetContext()
{
    Assert_NotNull(currentGraphicsContext.get());
    return ThreadSafeSharedView(currentGraphicsContextMutex, *currentGraphicsContext.get());
}

ThreadSafeExclusiveView<BaseGraphicsContext> GetMutableContext()
{
    Assert_NotNull(currentGraphicsContext.get());
    return ThreadSafeExclusiveView(currentGraphicsContextMutex, *currentGraphicsContext.get());
}

ApiLifetime::~ApiLifetime()
{
    Internal::ShutdownContext();
}

} // namespace Engine::Graphics
