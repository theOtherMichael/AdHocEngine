#pragma once

#include "../../BaseGraphicsContext.h"
#include <Engine/Core/SymbolExportMacros.h>

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

namespace Engine::Graphics::Platform
{

class MetalGraphicsContext : public BaseGraphicsContext
{
public:
    ENGINE_API MetalGraphicsContext();
    ENGINE_API ~MetalGraphicsContext();

    ENGINE_API void OnFramebufferResize(int width, int height) override;
    ENGINE_API void Present() const override;

    MTL::Device* device             = nullptr;
    MTL::CommandQueue* commandQueue = nullptr;
    CA::MetalLayer* metalLayer      = nullptr;

    mutable NS::AutoreleasePool* currentAutoreleasePool            = nullptr;
    mutable CA::MetalDrawable* currentDrawable                     = nullptr;
    mutable MTL::CommandBuffer* currentCommandBuffer               = nullptr;
    mutable MTL::RenderPassDescriptor* currentRenderPassDescriptor = nullptr;
};

} // namespace Engine::Graphics::Platform
