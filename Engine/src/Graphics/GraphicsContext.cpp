#include <Engine/Graphics/GraphicsContext.h>

#include <Engine/Core/Assertions.h>
#include <Engine/Graphics/Contexts/BaseGraphicsContext.h>
#include <Engine/Graphics/Contexts/D3d11GraphicsContext.h>
#include <Engine/Graphics/Contexts/D3d12GraphicsContext.h>
#include <Engine/Graphics/Contexts/MetalGraphicsContext.h>
#include <Engine/Graphics/Contexts/OpenGlGraphicsContext.h>
#include <Engine/Graphics/Contexts/VulkanGraphicsContext.h>

#include <memory>

namespace Engine::Graphics
{

static auto currentGraphicsContext = std::unique_ptr<BaseGraphicsContext>{};

BorrowHandle<BaseGraphicsContext> GetContext()
{
    Assert_NotNull(currentGraphicsContext.get());
    return BorrowHandle<BaseGraphicsContext>(currentGraphicsContext.get());
}

#if PLATFORM_SUPPORTS_OPENGL
BorrowHandle<OpenGlGraphicsContext> GetOpenGlContext()
{
    auto* ctx = dynamic_cast<OpenGlGraphicsContext*>(currentGraphicsContext.get());
    Assert_NotNull_Fmt(ctx, "OpenGL context is not active. Current API mode: {}", GetActiveApiMode());
    return BorrowHandle<OpenGlGraphicsContext>(ctx);
}
#endif

#if PLATFORM_SUPPORTS_VULKAN
BorrowHandle<VulkanGraphicsContext> GetVulkanContext()
{
    auto* ctx = dynamic_cast<VulkanGraphicsContext*>(currentGraphicsContext.get());
    Assert_NotNull_Fmt(ctx, "Vulkan context is not active. Current API mode: {}", GetActiveApiMode());
    return BorrowHandle<VulkanGraphicsContext>(ctx);
}
#endif

#if PLATFORM_SUPPORTS_D3D11
BorrowHandle<D3d11GraphicsContext> GetD3d11Context()
{
    auto* ctx = dynamic_cast<D3d11GraphicsContext*>(currentGraphicsContext.get());
    Assert_NotNull_Fmt(ctx, "D3D11 context is not active. Current API mode: {}", GetActiveApiMode());
    return BorrowHandle<D3d11GraphicsContext>(ctx);
}
#endif

#if PLATFORM_SUPPORTS_D3D12
BorrowHandle<D3d12GraphicsContext> GetD3d12Context()
{
    auto* ctx = dynamic_cast<D3d12GraphicsContext*>(currentGraphicsContext.get());
    Assert_NotNull_Fmt(ctx, "D3D12 context is not active. Current API mode: {}", GetActiveApiMode());
    return BorrowHandle<D3d12GraphicsContext>(ctx);
}
#endif

#if PLATFORM_SUPPORTS_METAL
BorrowHandle<MetalGraphicsContext> GetMetalContext()
{
    auto* ctx = dynamic_cast<MetalGraphicsContext*>(currentGraphicsContext.get());
    Assert_NotNull_Fmt(ctx, "Metal context is not active. Current API mode: {}", GetActiveApiMode());
    return BorrowHandle<MetalGraphicsContext>(ctx);
}
#endif

namespace Internal
{

static void LogContextCreatedSuccessfully(const ApiMode apiMode)
{
    Console::Log("Graphics context created successfully. API: {}", apiMode);
}

static void LogContextDestroyedSuccessfully(const ApiMode apiMode)
{
    Console::Log("Graphics context destroyed successfully. API: {}", apiMode);
}

static void LogInvalidApiError(const ApiMode apiMode)
{
    Console::LogError("API mode {} is not allowed.", apiMode);
}

static void LogUnsupportedApiError(const ApiMode apiMode)
{
    Console::LogError("API mode {} is not supported on this platform.", apiMode);
}

void CreateContext(ApiMode apiMode)
{
    switch (apiMode)
    {
    case Engine::Graphics::ApiMode::Uninitialized:
        LogInvalidApiError(apiMode);
        break;
    case Engine::Graphics::ApiMode::OpenGl:
    {
#if PLATFORM_SUPPORTS_OPENGL
        currentGraphicsContext = std::make_unique<OpenGlGraphicsContext>();
        LogContextCreatedSuccessfully(apiMode);
#else
        LogUnsupportedApiError(apiMode);
#endif
        break;
    }
    case Engine::Graphics::ApiMode::Vulkan:
    {
#if PLATFORM_SUPPORTS_VULKAN
        currentGraphicsContext = std::make_unique<VulkanGraphicsContext>();
        LogContextCreatedSuccessfully(apiMode);
#else
        LogUnsupportedApiError(apiMode);
#endif
        break;
    }
    case Engine::Graphics::ApiMode::D3d11:
    {
#if PLATFORM_SUPPORTS_D3D11
        currentGraphicsContext = std::make_unique<D3d11GraphicsContext>();
        LogContextCreatedSuccessfully(apiMode);
#else
        LogUnsupportedApiError(apiMode);
#endif
        break;
    }
    case Engine::Graphics::ApiMode::D3d12:
    {
#if PLATFORM_SUPPORTS_D3D12
        currentGraphicsContext = std::make_unique<D3d12GraphicsContext>();
        LogContextCreatedSuccessfully(apiMode);
#else
        LogUnsupportedApiError(apiMode);
#endif
        break;
    }
    case Engine::Graphics::ApiMode::Metal:
    {
#if PLATFORM_SUPPORTS_METAL
        currentGraphicsContext = std::make_unique<MetalGraphicsContext>();
        LogContextCreatedSuccessfully(apiMode);
#else
        LogUnsupportedApiError(apiMode);
#endif
        break;
    }
    default:
        Assert_NoEntry();
        break;
    }
}

void DestroyContext()
{
    const auto currentApi = GetActiveApiMode();
    currentGraphicsContext.reset(nullptr);
    LogContextDestroyedSuccessfully(currentApi);
}

} // namespace Internal

} // namespace Engine::Graphics
