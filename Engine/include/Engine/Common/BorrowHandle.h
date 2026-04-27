#pragma once

#include <Engine/Core/Assertions.h>

namespace Engine
{

template <typename T>
class BorrowHandle
{
public:
    BorrowHandle()                               = delete;
    BorrowHandle(const BorrowHandle&)            = delete;
    BorrowHandle& operator=(const BorrowHandle&) = delete;
    BorrowHandle(BorrowHandle&&)                 = delete;
    BorrowHandle& operator=(BorrowHandle&&)      = delete;

    explicit BorrowHandle(T* ptr) noexcept : m_ptr(ptr) {}

    [[nodiscard]] bool HasValue() const noexcept { return m_ptr != nullptr; }

    explicit operator bool() const noexcept { return HasValue(); }

    T& operator*() const
    {
        Assert_NotNull(m_ptr);
        return *m_ptr;
    }

    T* operator->() const
    {
        Assert_NotNull(m_ptr);
        return m_ptr;
    }

    T* Get() const noexcept { return m_ptr; }

private:
    T* m_ptr;
};

} // namespace Engine
