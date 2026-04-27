#pragma once

#include "../../BaseGraphicsContext.h"
#include <Engine/Core/SymbolExportMacros.h>

#include <d3d11.h>

namespace Engine::Graphics::Platform
{

class ENGINE_API D3d11GraphicsContext : public BaseGraphicsContext
{
public:
    D3d11GraphicsContext();
    ~D3d11GraphicsContext();

    void OnFramebufferResize(int width, int height) override;
    void Present() const override;

    ID3D11Device* pd3dDevice                     = nullptr;
    ID3D11DeviceContext* pd3dDeviceContext       = nullptr;
    IDXGISwapChain* pSwapChain                   = nullptr;
    ID3D11RenderTargetView* mainRenderTargetView = nullptr;
};

} // namespace Engine::Graphics::Platform
