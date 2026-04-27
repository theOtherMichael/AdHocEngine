#pragma once

#include "BaseGraphicsContext.h"
#include <Engine/Core/PlatformAbstraction.h>

#if PLATFORM_HEADER_EXISTS(D3d11GraphicsContext.h)
#define PLATFORM_SUPPORTS_D3D11 1
#include PLATFORM_HEADER(D3d11GraphicsContext.h)
#else
#define PLATFORM_SUPPORTS_D3D11 0
#endif

#include <concepts>

#if PLATFORM_SUPPORTS_D3D11

namespace Engine::Graphics
{

template <typename T>
concept IsD3d11GraphicsContext = requires(T t, int w, int h) {
    requires std::derived_from<T, BaseGraphicsContext>;

    { t.pd3dDevice } -> std::convertible_to<ID3D11Device*>;
    { t.pd3dDeviceContext } -> std::convertible_to<ID3D11DeviceContext*>;
    { t.pSwapChain } -> std::convertible_to<IDXGISwapChain*>;
    { t.mainRenderTargetView } -> std::convertible_to<ID3D11RenderTargetView*>;
};

static_assert(IsD3d11GraphicsContext<Platform::D3d11GraphicsContext>);
using D3d11GraphicsContext = Platform::D3d11GraphicsContext;

} // namespace Engine::Graphics

#endif // PLATFORM_SUPPORTS_D3D11
