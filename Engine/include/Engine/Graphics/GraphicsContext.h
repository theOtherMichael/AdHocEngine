#pragma once
#pragma once

#include <Engine/Common/ThreadSafeViews.h>
#include <Engine/Core/SymbolExportMacros.h>
#include <Engine/Graphics/_platform/Base/BaseGraphicsContext.h>

#include <vector>

namespace Engine::Graphics
{

enum class ApiMode
{
    None,
    OpenGl,
    Vulkan,
    D3D11,
    D3D12,
    Metal,
    // TODO: Handle extensions
};

ENGINE_API bool SetApiMode(ApiMode apiMode);

ENGINE_API ApiMode GetApiMode();

namespace Internal
{
ENGINE_API BaseGraphicsContext& GetContextRaw();
ENGINE_API std::shared_mutex& GetContextMutex();
} // namespace Internal

ENGINE_API ThreadSafeSharedView<BaseGraphicsContext> GetContext();

template <typename DerivedContextType>
    requires std::is_base_of_v<BaseGraphicsContext, DerivedContextType>
ThreadSafeSharedView<DerivedContextType> GetContextAs()
{
    const auto& context = dynamic_cast<const DerivedContextType&>(Internal::GetContextRaw());
    return ThreadSafeSharedView(Internal::GetContextMutex(), context);
}

ENGINE_API ThreadSafeExclusiveView<BaseGraphicsContext> GetMutableContext();

template <typename DerivedContextType>
    requires std::is_base_of_v<BaseGraphicsContext, DerivedContextType>
ThreadSafeExclusiveView<DerivedContextType> GetMutableContextAs()
{
    auto& context = dynamic_cast<DerivedContextType&>(Internal::GetContextRaw());
    return ThreadSafeExclusiveView(Internal::GetContextMutex(), context);
}

class ENGINE_API ApiLifetime
{
public:
    ApiLifetime()                              = default;
    ApiLifetime(const ApiLifetime&)            = delete;
    ApiLifetime& operator=(const ApiLifetime&) = delete;
    ~ApiLifetime();
};

} // namespace Engine::Graphics
