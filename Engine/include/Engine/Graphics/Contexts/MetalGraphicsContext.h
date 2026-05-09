#pragma once

#include "BaseGraphicsContext.h"
#include <Engine/Core/PlatformAbstraction.h>

#if PLATFORM_HEADER_EXISTS(MetalGraphicsContext.h)
#define PLATFORM_SUPPORTS_METAL 1
#include PLATFORM_HEADER(MetalGraphicsContext.h)
#else
#define PLATFORM_SUPPORTS_METAL 0
#endif

#include <concepts>

#if PLATFORM_SUPPORTS_METAL

namespace Engine::Graphics
{

template <typename T>
concept IsMetalGraphicsContext = requires(T t, int w, int h) {
    requires std::derived_from<T, BaseGraphicsContext>;

    { t.device }       -> std::convertible_to<MTL::Device*>;
    { t.commandQueue } -> std::convertible_to<MTL::CommandQueue*>;
    { t.metalLayer }   -> std::convertible_to<CA::MetalLayer*>;

    { t.currentDrawable }             -> std::convertible_to<CA::MetalDrawable*>;
    { t.currentCommandBuffer }        -> std::convertible_to<MTL::CommandBuffer*>;
    { t.currentRenderPassDescriptor } -> std::convertible_to<MTL::RenderPassDescriptor*>;
};

static_assert(IsMetalGraphicsContext<Platform::MetalGraphicsContext>);
using MetalGraphicsContext = Platform::MetalGraphicsContext;

} // namespace Engine::Graphics

#endif // PLATFORM_SUPPORTS_METAL
