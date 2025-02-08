#pragma once

#include "../Base/BaseGraphicsContext.h"
#include <Engine/Core/SymbolExportMacros.h>

#include <d3d11.h>

namespace Engine::Graphics
{

class ENGINE_API WindowsD3D11GraphicsContext : public BaseGraphicsContext
{
public:
    ID3D11Device* pd3dDevice                     = nullptr;
    ID3D11DeviceContext* pd3dDeviceContext       = nullptr;
    IDXGISwapChain* pSwapChain                   = nullptr;
    ID3D11RenderTargetView* mainRenderTargetView = nullptr;

    void OnFramebufferResize(int width, int height) override;
    void Present() const override;

    WindowsD3D11GraphicsContext();
    WindowsD3D11GraphicsContext(const WindowsD3D11GraphicsContext&)            = delete;
    WindowsD3D11GraphicsContext& operator=(const WindowsD3D11GraphicsContext&) = delete;
    ~WindowsD3D11GraphicsContext();
};

typedef WindowsD3D11GraphicsContext D3D11GraphicsContext;

} // namespace Engine::Graphics
