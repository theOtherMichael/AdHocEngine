#pragma once

#include "Contexts/BaseGraphicsContext.h"
#include "Contexts/D3d11GraphicsContext.h"
#include "Contexts/D3d12GraphicsContext.h"
#include "Contexts/MetalGraphicsContext.h"
#include "Contexts/OpenGlGraphicsContext.h"
#include "Contexts/VulkanGraphicsContext.h"

#include <Engine/Common/BorrowHandle.h>
#include <Engine/Core/SymbolExportMacros.h>
#include <Engine/Graphics/ApiMode.h>

namespace Engine::Graphics
{

ENGINE_API BorrowHandle<BaseGraphicsContext> GetContext();

#if PLATFORM_SUPPORTS_OPENGL
ENGINE_API BorrowHandle<OpenGlGraphicsContext> GetOpenGlContext();
#endif
#if PLATFORM_SUPPORTS_VULKAN
ENGINE_API BorrowHandle<VulkanGraphicsContext> GetVulkanContext();
#endif
#if PLATFORM_SUPPORTS_D3D11
ENGINE_API BorrowHandle<D3d11GraphicsContext> GetD3d11Context();
#endif
#if PLATFORM_SUPPORTS_D3D12
ENGINE_API BorrowHandle<D3d12GraphicsContext> GetD3d12Context();
#endif
#if PLATFORM_SUPPORTS_METAL
ENGINE_API BorrowHandle<MetalGraphicsContext> GetMetalContext();
#endif

namespace Internal
{
ENGINE_API void CreateContext(ApiMode api);
ENGINE_API void DestroyContext();
ENGINE_API BaseGraphicsContext& GetContextUnsafe();
} // namespace Internal

} // namespace Engine::Graphics
