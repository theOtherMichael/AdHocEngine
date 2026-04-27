#pragma once

#include "BaseGraphicsContext.h"
#include <Engine/Core/PlatformAbstraction.h>

#if PLATFORM_HEADER_EXISTS(OpenGlGraphicsContext.h)
#define PLATFORM_SUPPORTS_OPENGL 1
#include PLATFORM_HEADER(OpenGlGraphicsContext.h)
#else
#define PLATFORM_SUPPORTS_OPENGL 0
#endif

#include <concepts>

#if PLATFORM_SUPPORTS_OPENGL

namespace Engine::Graphics
{

template <typename T>
concept IsOpenGlGraphicsContext = requires(T t, int w, int h) {
    requires std::derived_from<T, BaseGraphicsContext>;

    // TODO: Other req's
};

static_assert(IsOpenGlGraphicsContext<Platform::OpenGlGraphicsContext>);
using OpenGlGraphicsContext = Platform::OpenGlGraphicsContext;

} // namespace Engine::Graphics

#endif // PLATFORM_SUPPORTS_OPENGL
