#pragma once

#include "../../BaseGraphicsContext.h"
#include <Engine/Core/SymbolExportMacros.h>

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

namespace Engine::Graphics::Platform
{

class ENGINE_API MetalGraphicsContext : public BaseGraphicsContext
{
public:
    MetalGraphicsContext();
    ~MetalGraphicsContext();

    void OnFramebufferResize(int width, int height) override;
    void Present() const override;

    MTL::Device* device             = nullptr;
    MTL::CommandQueue* commandQueue = nullptr;
    CA::MetalLayer* metalLayer      = nullptr;

    mutable NS::AutoreleasePool* currentAutoreleasePool            = nullptr;
    mutable CA::MetalDrawable* currentDrawable                     = nullptr;
    mutable MTL::CommandBuffer* currentCommandBuffer               = nullptr;
    mutable MTL::RenderPassDescriptor* currentRenderPassDescriptor = nullptr;
};

} // namespace Engine::Graphics::Platform
