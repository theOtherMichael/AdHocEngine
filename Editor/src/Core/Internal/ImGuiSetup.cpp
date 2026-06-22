#include <Editor/Core/Internal/ImGuiSetup.h>

#include <Engine/Core/Assertions.h>
#include <Engine/Core/Console.h>
#include <Engine/File/SystemPaths.h>
#include <Engine/Graphics/Contexts/D3d11GraphicsContext.h>
#include <Engine/Graphics/Contexts/D3d12GraphicsContext.h>
#include <Engine/Graphics/Contexts/MetalGraphicsContext.h>
#include <Engine/Graphics/Contexts/OpenGlGraphicsContext.h>
#include <Engine/Graphics/Contexts/VulkanGraphicsContext.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Window/WindowState.h>

#include <imgui.h>
#if PLATFORM_SUPPORTS_D3D11
#include <backends/imgui_impl_dx11.h>
#endif
#if PLATFORM_SUPPORTS_METAL
#include <backends/imgui_impl_metal.h>
#endif
#include <backends/imgui_impl_glfw.h>

#include <string>

namespace Console = Engine::Console;
using Engine::Graphics::ApiMode;

namespace Editor::Internal
{

static void LogImGuiContextCreatedSuccessfully(const ApiMode apiMode)
{
    Console::Log("ImGui initialized successfully. API: {}", apiMode);
}

static void LogImGuiContextShutdownSuccessfully(const ApiMode apiMode)
{
    Console::Log("ImGui shutdown successfully. API: {}", apiMode);
}

void InitializeImGui()
{
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();

    static const auto imguiIniFilePath = (Engine::GetExecutablePath().parent_path() / "imgui.ini").string();

    auto& io        = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.IniFilename  = imguiIniFilePath.c_str();

    const auto& windowState  = Engine::Window::WindowState::Instance();
    auto* const windowHandle = windowState.mainWindowHandle;

    const auto currentApi = Engine::Graphics::GetActiveApiMode();
    switch (currentApi)
    {
    case ApiMode::OpenGl:
#if PLATFORM_SUPPORTS_OPENGL
    {
        // TODO: Implement ImGui + OpenGL
        const auto openGlContext = Engine::Graphics::GetContextAs<Engine::Graphics::OpenGlGraphicsContext>();
        break;
    }
#else
        Console::LogError("OpenGL is not supported.");
        // TODO: Throw exception
        break;
#endif
    case ApiMode::Vulkan:
#if PLATFORM_SUPPORTS_VULKAN
    {
        // TODO: Implement ImGui + Vulkan
        const auto vulkanContext = Engine::Graphics::GetContextAs<Engine::Graphics::VulkanGraphicsContext>();
        break;
    }
#else
        Console::LogError("Vulkan is not supported.");
        // TODO: Throw exception
        break;
#endif
    case ApiMode::D3d11:
#if PLATFORM_SUPPORTS_D3D11
    {
        const auto d3d11Context = Engine::Graphics::GetD3d11Context();
        ImGui_ImplGlfw_InitForOther(windowHandle, true);
        ImGui_ImplDX11_Init(d3d11Context->pd3dDevice, d3d11Context->pd3dDeviceContext);
        LogImGuiContextCreatedSuccessfully(currentApi);
        break;
    }
#else
        Console::LogError("D3D11 is not supported.");
        // TODO: Throw exception
        break;
#endif
    case ApiMode::D3d12:
#if PLATFORM_SUPPORTS_D3D12
    {
        // TODO: Implement ImGui + D3D12
        const auto d3d12Context = Engine::Graphics::GetContextAs<Engine::Graphics::D3d12GraphicsContext>();
        break;
    }
#else
        Console::LogError("D3D12 is not supported.");
        // TODO: Throw exception
        break;
#endif
    case ApiMode::Metal:
#if PLATFORM_SUPPORTS_METAL
    {
        const auto metalContext = Engine::Graphics::GetMetalContext();
        ImGui_ImplGlfw_InitForOther(windowHandle, true);
        ImGui_ImplMetal_Init(metalContext->device);
        LogImGuiContextCreatedSuccessfully(currentApi);
        break;
    }
#else
        Console::LogError("Metal is not supported.");
        break;
#endif
    case ApiMode::Uninitialized:
        [[fallthrough]];
    default:
        Assert_NoEntry();
    }
}

void ShutdownImGui()
{
    const auto currentApi = Engine::Graphics::GetActiveApiMode();
    switch (currentApi)
    {
    case ApiMode::OpenGl:
#if PLATFORM_SUPPORTS_OPENGL
        // TODO: Implement ImGui + OpenGL
        Console::LogError("OpenGL is not implemented.");
#else
        Console::LogError("OpenGL is not supported.");
#endif
        break;
    case ApiMode::Vulkan:
#if PLATFORM_SUPPORTS_VULKAN
        // TODO: Implement ImGui + Vulkan
        Console::LogError("Vulkan is not implemented.");
#else
        Console::LogError("Vulkan is not supported.");
#endif
        break;
    case ApiMode::D3d11:
#if PLATFORM_SUPPORTS_D3D11
        ImGui_ImplDX11_Shutdown();
        LogImGuiContextShutdownSuccessfully(currentApi);
#else
        Console::LogError("D3D11 is not supported.");
#endif
        break;
    case ApiMode::D3d12:
#if PLATFORM_SUPPORTS_D3D12
        // TODO: Implement ImGui + D3D12
        Console::LogError("D3D12 is not implemented.");
#else
        Console::LogError("D3D12 is not supported.");
#endif
        break;
    case ApiMode::Metal:
#if PLATFORM_SUPPORTS_METAL
        ImGui_ImplMetal_Shutdown();
        LogImGuiContextShutdownSuccessfully(currentApi);
#else
        Console::LogError("Metal is not supported.");
#endif
        break;
    case ApiMode::Uninitialized:
        [[fallthrough]];
    default:
        Assert_NoEntry();
    }

    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void StartImGuiFrame()
{
    const auto currentApi = Engine::Graphics::GetActiveApiMode();
    switch (currentApi)
    {
    case ApiMode::OpenGl:
#if PLATFORM_SUPPORTS_OPENGL
        Console::LogError("OpenGL is not implemented.");
#else
        Console::LogError("OpenGL is not supported.");
#endif
        break;
    case ApiMode::Vulkan:
#if PLATFORM_SUPPORTS_VULKAN
        Console::LogError("Vulkan is not implemented.");
#else
        Console::LogError("Vulkan is not supported.");
#endif
        break;
    case ApiMode::D3d11:
#if PLATFORM_SUPPORTS_D3D11
    {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        break;
    }
#else
        Console::LogError("D3D11 is not supported.");
        break;
#endif
    case ApiMode::D3d12:
#if PLATFORM_SUPPORTS_D3D12
        Console::LogError("D3D12 is not implemented.");
#else
        Console::LogError("D3D12 is not supported.");
#endif
        break;
    case ApiMode::Metal:
#if PLATFORM_SUPPORTS_METAL
    {
        const auto metalContext = Engine::Graphics::GetMetalContext();

        metalContext->currentAutoreleasePool = NS::AutoreleasePool::alloc()->init();

        auto* drawable = metalContext->metalLayer->nextDrawable();
        drawable->retain();
        metalContext->currentDrawable = drawable;

        auto* commandBuffer = metalContext->commandQueue->commandBuffer();
        commandBuffer->retain();
        metalContext->currentCommandBuffer = commandBuffer;

        auto* rpd = MTL::RenderPassDescriptor::renderPassDescriptor();
        rpd->retain();
        metalContext->currentRenderPassDescriptor = rpd;

        auto* colorAttachment = rpd->colorAttachments()->object(0);
        colorAttachment->setTexture(drawable->texture());
        colorAttachment->setLoadAction(MTL::LoadActionClear);
        colorAttachment->setClearColor(MTL::ClearColor::Make(0.1, 0.1, 0.1, 1.0));
        colorAttachment->setStoreAction(MTL::StoreActionStore);

        ImGui_ImplMetal_NewFrame(rpd);
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        break;
    }
#else
        Console::LogError("Metal is not supported.");
        break;
#endif
    case ApiMode::Uninitialized:
        [[fallthrough]];
    default:
        Assert_NoEntry();
    }
}

void EndImGuiFrame()
{
    ImGui::Render();

    const auto currentApi = Engine::Graphics::GetActiveApiMode();
    switch (currentApi)
    {
    case ApiMode::OpenGl:
#if PLATFORM_SUPPORTS_OPENGL
        Console::LogError("OpenGL is not implemented.");
#else
        Console::LogError("OpenGL is not supported.");
#endif
        break;
    case ApiMode::Vulkan:
#if PLATFORM_SUPPORTS_VULKAN
        Console::LogError("Vulkan is not implemented.");
#else
        Console::LogError("Vulkan is not supported.");
#endif
        break;
    case ApiMode::D3d11:
#if PLATFORM_SUPPORTS_D3D11
    {
        auto d3d11Context = Engine::Graphics::GetD3d11Context();

        const auto clearColor = std::array{0.1f, 0.1f, 0.1f, 1.0f};
        d3d11Context->pd3dDeviceContext->OMSetRenderTargets(1, &d3d11Context->mainRenderTargetView, nullptr);
        d3d11Context->pd3dDeviceContext->ClearRenderTargetView(d3d11Context->mainRenderTargetView, clearColor.data());
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        PumpImGuiPlatformWindows();

        d3d11Context->Present();
        break;
    }
#else
        Console::LogError("D3D11 is not supported.");
        break;
#endif
    case ApiMode::D3d12:
#if PLATFORM_SUPPORTS_D3D12
        Console::LogError("D3D12 is not implemented.");
#else
        Console::LogError("D3D12 is not supported.");
#endif
        break;
    case ApiMode::Metal:
#if PLATFORM_SUPPORTS_METAL
    {
        const auto metalContext = Engine::Graphics::GetMetalContext();
        auto* encoder =
            metalContext->currentCommandBuffer->renderCommandEncoder(metalContext->currentRenderPassDescriptor);
        ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), metalContext->currentCommandBuffer, encoder);
        encoder->endEncoding();

        PumpImGuiPlatformWindows();

        metalContext->Present();
        break;
    }
#else
        Console::LogError("Metal is not supported.");
        break;
#endif
    case ApiMode::Uninitialized:
        [[fallthrough]];
    default:
        Assert_NoEntry();
    }
}

void PumpImGuiPlatformWindows()
{
    auto& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

} // namespace Editor::Internal
