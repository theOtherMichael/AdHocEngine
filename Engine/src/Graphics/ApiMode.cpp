#include <Engine/Graphics/ApiMode.h>

#include <Engine/Core/Assertions.h>
#include <Engine/Core/Console.h>
#include <Engine/Graphics/GraphicsContext.h>

namespace Engine::Graphics
{

static auto currentApiMode = ApiMode::Uninitialized;

ApiMode GetActiveApiMode()
{
    return currentApiMode;
}

void SetApiMode(ApiMode apiMode)
{
    // TODO: Graphics API change requested event

    Internal::CreateContext(apiMode);
    currentApiMode = apiMode;

    // TODO: Graphics API change completed event
}

} // namespace Engine::Graphics
