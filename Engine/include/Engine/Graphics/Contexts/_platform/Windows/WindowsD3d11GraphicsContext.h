#pragma once

#include "../../BaseGraphicsContext.h"
#include <Engine/Core/SymbolExportMacros.h>

#include <d3d11.h>

namespace Engine::Graphics::Platform
{

class D3d11GraphicsContext : public BaseGraphicsContext
{
public:
    ENGINE_API D3d11GraphicsContext();
    ENGINE_API ~D3d11GraphicsContext();

    ENGINE_API void OnFramebufferResize(int width, int height) override;
    ENGINE_API void Present() const override;

    ID3D11Device* pd3dDevice                     = nullptr;
    ID3D11DeviceContext* pd3dDeviceContext       = nullptr;
    IDXGISwapChain* pSwapChain                   = nullptr;
    ID3D11RenderTargetView* mainRenderTargetView = nullptr;
};

} // namespace Engine::Graphics::Platform
