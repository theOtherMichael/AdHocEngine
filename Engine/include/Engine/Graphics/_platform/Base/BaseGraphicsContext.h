#pragma once

namespace Engine::Graphics
{

class BaseGraphicsContext
{
public:
    virtual void OnFramebufferResize(int width, int height) = 0;
    virtual void Present() const                            = 0;

    BaseGraphicsContext()                                      = default;
    BaseGraphicsContext(const BaseGraphicsContext&)            = delete;
    BaseGraphicsContext& operator=(const BaseGraphicsContext&) = delete;
    virtual ~BaseGraphicsContext()                             = default;
};

} // namespace Engine::Graphics
