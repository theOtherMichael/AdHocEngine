#pragma once

namespace Engine::Graphics
{

class BaseGraphicsContext
{
public:
    BaseGraphicsContext()                                      = default;
    virtual ~BaseGraphicsContext()                             = default;
    BaseGraphicsContext(const BaseGraphicsContext&)            = delete;
    BaseGraphicsContext& operator=(const BaseGraphicsContext&) = delete;

    virtual void OnFramebufferResize(int width, int height) = 0;
    virtual void Present() const                            = 0;
};

} // namespace Engine::Graphics
