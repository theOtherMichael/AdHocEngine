#include <Engine/Graphics/Contexts/_platform/Windows/WindowsD3D11GraphicsContext.h>

#include <Engine/Core/Assertions.h>
#include <Engine/Core/Console.h>
#include <Engine/Window/WindowState.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#pragma clang diagnostic pop

#include <array>

static_assert(ADHOC_WINDOWS);

namespace Engine::Graphics::Platform
{

void D3d11GraphicsContext::OnFramebufferResize(int width, int height)
{
    if (!pSwapChain)
        return;

    mainRenderTargetView->Release();
    pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

    auto* pBackBuffer = static_cast<ID3D11Texture2D*>(nullptr);
    pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    Assert_NotNull(pBackBuffer);
    pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &mainRenderTargetView);
    pBackBuffer->Release();
}

void D3d11GraphicsContext::Present() const
{
    pSwapChain->Present(1, 0);
}

D3d11GraphicsContext::D3d11GraphicsContext()
{
    const auto mainWindowHandle   = Window::WindowState::Instance().mainWindowHandle;
    const auto nativeWindowHandle = glfwGetWin32Window(mainWindowHandle);

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

    auto featureLevel            = D3D_FEATURE_LEVEL{};
    constexpr auto featureLevels = std::array{D3D_FEATURE_LEVEL_11_0};

    if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr,
                                             D3D_DRIVER_TYPE_HARDWARE,
                                             nullptr,
                                             0,
                                             featureLevels.data(),
                                             static_cast<UINT>(featureLevels.size()),
                                             D3D11_SDK_VERSION,
                                             &sd,
                                             &pSwapChain,
                                             &pd3dDevice,
                                             &featureLevel,
                                             &pd3dDeviceContext)))
    {
        Console::LogFatal("D3D11 device and swap chain could not be created.");
    }

    auto pBackBuffer = static_cast<ID3D11Texture2D*>(nullptr);
    pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    Assert_NotNull(pBackBuffer);
    pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &mainRenderTargetView);
    pBackBuffer->Release();
}

D3d11GraphicsContext::~D3d11GraphicsContext()
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

} // namespace Engine::Graphics::Platform
