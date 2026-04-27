#pragma once

#include <Engine/Core/Formatters/EnumFormatter.h>
#include <Engine/Core/SymbolExportMacros.h>

namespace Engine::Graphics
{

INJECT_ENUM_FORMATTER

enum class ApiMode
{
    Uninitialized,
    OpenGl,
    Vulkan,
    D3d11,
    D3d12,
    Metal,
};

ENGINE_API ApiMode GetActiveApiMode();

ENGINE_API void SetApiMode(ApiMode apiMode);

} // namespace Engine::Graphics
