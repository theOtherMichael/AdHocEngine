#include <Engine/Graphics/_platform/Windows/WindowsD3D11GraphicsContext.h>

#include <Engine/Core/Assertions.h>
#include <Engine/Core/Console.h>
#include <Engine/Core/PlatformData.h>
#include <Engine/Window/WindowData.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#pragma clang diagnostic pop

ASSERT_PLATFORM_WINDOWS;

namespace Engine::Graphics
{

void WindowsD3D11GraphicsContext::OnFramebufferResize(int width, int height)
{
    if (!pSwapChain)
        return;

    mainRenderTargetView->Release();
    pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

    ID3D11Texture2D* pBackBuffer;
    pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &mainRenderTargetView);
    pBackBuffer->Release();
}

void WindowsD3D11GraphicsContext::Present() const
{
    pSwapChain->Present(1, 0);
}

WindowsD3D11GraphicsContext::WindowsD3D11GraphicsContext()
{
    auto nativeWindowHandle = glfwGetWin32Window(Window::WindowData::GetInstance().mainWindowHandle);

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount                        = 2;
    sd.BufferDesc.Width                   = 0;
    sd.BufferDesc.Height                  = 0;
    sd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator   = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow                       = nativeWindowHandle;
    sd.SampleDesc.Count                   = 1;
    sd.SampleDesc.Quality                 = 0;
    sd.Windowed                           = TRUE;
    sd.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_0};

    if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr,
                                             D3D_DRIVER_TYPE_HARDWARE,
                                             nullptr,
                                             0,
                                             featureLevels,
                                             1,
                                             D3D11_SDK_VERSION,
                                             &sd,
                                             &pSwapChain,
                                             &pd3dDevice,
                                             &featureLevel,
                                             &pd3dDeviceContext)))
    {
        Console::LogFatal("Failed to create D3D11 device and swap chain!");
    }

    ID3D11Texture2D* pBackBuffer;
    pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &mainRenderTargetView);
    pBackBuffer->Release();
}

WindowsD3D11GraphicsContext::~WindowsD3D11GraphicsContext()
{
    if (mainRenderTargetView)
    {
        mainRenderTargetView->Release();
        mainRenderTargetView = nullptr;
    }
    if (pSwapChain)
    {
        pSwapChain->Release();
        pSwapChain = nullptr;
    }
    if (pd3dDeviceContext)
    {
        pd3dDeviceContext->Release();
        pd3dDeviceContext = nullptr;
    }
    if (pd3dDevice)
    {
        pd3dDevice->Release();
        pd3dDevice = nullptr;
    }
}

} // namespace Engine::Graphics
