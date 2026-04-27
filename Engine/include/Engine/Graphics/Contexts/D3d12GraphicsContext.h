#pragma once

#include "BaseGraphicsContext.h"
#include <Engine/Core/PlatformAbstraction.h>

#if PLATFORM_HEADER_EXISTS(D3D12GraphicsContext.h)
#define PLATFORM_SUPPORTS_D3D12 1
#include PLATFORM_HEADER(D3D12GraphicsContext.h)
#else
#define PLATFORM_SUPPORTS_D3D12 0
#endif

#include <concepts>

#if PLATFORM_SUPPORTS_D3D12

namespace Engine::Graphics
{

template <typename T>
concept IsD3d12GraphicsContext = requires(T t, int w, int h) {
    requires std::derived_from<T, BaseGraphicsContext>;

    // TODO: Other req's
};

static_assert(IsD3d12GraphicsContext<Platform::D3d12GraphicsContext>);
using D3D12GraphicsContext = Platform::D3D12GraphicsContext;

} // namespace Engine::Graphics

#endif // PLATFORM_SUPPORTS_D3D12
