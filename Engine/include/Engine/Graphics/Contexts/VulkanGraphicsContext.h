#pragma once

#include "BaseGraphicsContext.h"
#include <Engine/Core/PlatformAbstraction.h>

#if PLATFORM_HEADER_EXISTS(VulkanGraphicsContext.h)
#define PLATFORM_SUPPORTS_VULKAN 1
#include PLATFORM_HEADER(VulkanGraphicsContext.h)
#else
#define PLATFORM_SUPPORTS_VULKAN 0
#endif

#include <concepts>

#if PLATFORM_SUPPORTS_VULKAN

namespace Engine::Graphics
{

template <typename T>
concept IsVulkanGraphicsContext = requires(T t, int w, int h) {
    requires std::derived_from<T, BaseGraphicsContext>;

    // TODO: Other req's
};

static_assert(IsVulkanGraphicsContext<Platform::VulkanGraphicsContext>);
using VulkanGraphicsContext = Platform::VulkanGraphicsContext;

} // namespace Engine::Graphics

#endif // PLATFORM_SUPPORTS_VULKAN
