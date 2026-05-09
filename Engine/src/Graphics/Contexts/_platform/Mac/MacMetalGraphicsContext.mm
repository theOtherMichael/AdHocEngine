#include <Engine/Graphics/Contexts/_platform/Mac/MacMetalGraphicsContext.h>

#include <Engine/Core/Console.h>
#include <Engine/Window/WindowState.h>

#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#pragma clang diagnostic pop

#import <AppKit/AppKit.h>

static_assert(ADHOC_MAC);

namespace Engine::Graphics::Platform
{

MetalGraphicsContext::MetalGraphicsContext()
{
    auto* const setupPool = NS::AutoreleasePool::alloc()->init();

    device = MTL::CreateSystemDefaultDevice();
    if (!device)
        Console::LogFatal("Metal device could not be created.");

    commandQueue = device->newCommandQueue();
    if (!commandQueue)
        Console::LogFatal("Metal command queue could not be created.");

    metalLayer = CA::MetalLayer::layer();
    metalLayer->retain();
    metalLayer->setDevice(device);
    metalLayer->setPixelFormat(MTL::PixelFormatBGRA8Unorm);

    const auto mainWindowHandle     = Engine::Window::WindowState::Instance().mainWindowHandle;
    NSWindow* nsWindow              = glfwGetCocoaWindow(mainWindowHandle);
    nsWindow.contentView.layer      = (__bridge CALayer*)(void*)metalLayer;
    nsWindow.contentView.wantsLayer = YES;

    auto width  = 0;
    auto height = 0;
    glfwGetFramebufferSize(mainWindowHandle, &width, &height);
    metalLayer->setDrawableSize({static_cast<CGFloat>(width), static_cast<CGFloat>(height)});

    setupPool->release();
}

MetalGraphicsContext::~MetalGraphicsContext()
{
    if (currentAutoreleasePool)
    {
        currentAutoreleasePool->release();
        currentAutoreleasePool = nullptr;
    }
    if (currentRenderPassDescriptor)
    {
        currentRenderPassDescriptor->release();
        currentRenderPassDescriptor = nullptr;
    }
    if (currentCommandBuffer)
    {
        currentCommandBuffer->release();
        currentCommandBuffer = nullptr;
    }
    if (currentDrawable)
    {
        currentDrawable->release();
        currentDrawable = nullptr;
    }
    if (commandQueue)
    {
        commandQueue->release();
        commandQueue = nullptr;
    }
    if (metalLayer)
    {
        metalLayer->release();
        metalLayer = nullptr;
    }
    if (device)
    {
        device->release();
        device = nullptr;
    }
}

void MetalGraphicsContext::OnFramebufferResize(int width, int height)
{
    metalLayer->setDrawableSize({static_cast<CGFloat>(width), static_cast<CGFloat>(height)});
}

void MetalGraphicsContext::Present() const
{
    currentCommandBuffer->presentDrawable(currentDrawable);
    currentCommandBuffer->commit();

    currentRenderPassDescriptor->release();
    currentRenderPassDescriptor = nullptr;

    currentCommandBuffer->release();
    currentCommandBuffer = nullptr;

    currentDrawable->release();
    currentDrawable = nullptr;

    if (currentAutoreleasePool)
    {
        currentAutoreleasePool->release();
        currentAutoreleasePool = nullptr;
    }
}

} // namespace Engine::Graphics::Platform
