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
    {
        auto lock = std::unique_lock(currentGraphicsContextMutex);
        currentGraphicsContext.reset(nullptr);
    }
    break;
    case Engine::Graphics::ApiMode::OpenGl: Assert_NoEntry(); break;
    case Engine::Graphics::ApiMode::Vulkan: Assert_NoEntry(); break;
    case Engine::Graphics::ApiMode::D3D11:
    {
#if ADHOC_WINDOWS
        auto lock = std::unique_lock(currentGraphicsContextMutex);
        currentGraphicsContext.reset(nullptr);
        currentGraphicsContext = std::make_unique<D3D11GraphicsContext>();
#else
        Assert_NoEntry();
#endif
    }
    break;
    case Engine::Graphics::ApiMode::D3D12: Assert_NoEntry(); break;
    case Engine::Graphics::ApiMode::Metal: Assert_NoEntry(); break;
    default: Assert_NoEntry(); break;
    }

    currentApiMode = apiMode;
    return true;
}

ApiMode GetApiMode()
{
    return currentApiMode;
}

BaseGraphicsContext& Internal::GetContextRaw()
{
    return *currentGraphicsContext.get();
}

std::shared_mutex& Internal::GetContextMutex()
{
    return currentGraphicsContextMutex;
}

ThreadSafeSharedView<BaseGraphicsContext> GetContext()
{
    return ThreadSafeSharedView(currentGraphicsContextMutex, *currentGraphicsContext.get());
}

ThreadSafeExclusiveView<BaseGraphicsContext> GetMutableContext()
{
    return ThreadSafeExclusiveView(currentGraphicsContextMutex, *currentGraphicsContext.get());
}

ApiLifetime::~ApiLifetime()
{
    SetApiMode(ApiMode::None);
}

} // namespace Engine::Graphics
