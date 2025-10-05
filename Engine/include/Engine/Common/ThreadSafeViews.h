#pragma once

#include <Engine/Core/Assertions.h>

#include <chrono>
#include <future>
#include <optional>
#include <shared_mutex>

namespace Engine
{

/// @brief A const object reference associated with a std::shared_lock.
template <typename T>
struct ThreadSafeSharedView
{
public:
    explicit ThreadSafeSharedView(std::shared_mutex& mutex, const T& value) : lock(mutex), value(value) {}

    const T& operator*() const { return value; }

    const T* operator->() const { return &value; }

private:
    std::shared_lock<std::shared_mutex> lock;
    const T& value;
};

/// @brief A mutable object reference associated with a std::unique_lock.
template <typename T>
struct ThreadSafeExclusiveView
{
public:
    explicit ThreadSafeExclusiveView(std::shared_mutex& mutex, T& value) : lock(mutex), value(value) {}

    T& operator*() const { return value; }

    T* operator->() const { return &value; }

private:
    std::unique_lock<std::shared_mutex> lock;
    T& value;
};

/// @brief A const object reference associated with a std::shared_lock.
template <typename T>
struct ThreadSafeSharedViewAsync
{
public:
    explicit ThreadSafeSharedViewAsync(std::shared_mutex& mutex, const T& value)
        : mutex(mutex),
          value(value),
          lockFuture(std::async(std::launch::async, [&mutex] { return std::shared_lock{mutex}; }))
    {}

    bool IsReady() const
    {
        if (!lockFuture.valid())
            return lock.has_value();

        return lockFuture.wait_for(std::chrono::seconds::zero()) == std::future_status::ready;
    }

    explicit operator bool() const { return IsReady(); }

    const T& operator*() const
    {
        EnsureLocked();
        return value;
    }

    const T* operator->() const
    {
        EnsureLocked();
        return &value;
    }

private:
    std::shared_mutex& mutex;
    const T& value;

    mutable std::future<std::shared_lock<std::shared_mutex>> lockFuture;
    mutable std::optional<std::shared_lock<std::shared_mutex>> lock;

    void EnsureLocked() const
    {
        if (!lock)
            lock = lockFuture.get();

        Assert_True(lock.has_value());
    }
};

/// @brief A mutable object reference associated with a std::unique_lock.
template <typename T>
struct ThreadSafeExclusiveViewAsync
{
public:
    explicit ThreadSafeExclusiveViewAsync(std::shared_mutex& mutex, T& value)
        : mutex(mutex),
          value(value),
          lockFuture(std::async(std::launch::async, [&mutex] { return std::unique_lock{mutex}; }))
    {}

    bool IsReady() const
    {
        if (!lockFuture.valid())
            return lock.has_value();

        return lockFuture.wait_for(std::chrono::seconds::zero()) == std::future_status::ready;
    }

    explicit operator bool() const { return IsReady(); }

    T& operator*()
    {
        EnsureLocked();
        return value;
    }

    T* operator->()
    {
        EnsureLocked();
        return &value;
    }

private:
    std::shared_mutex& mutex;
    T& value;

    std::future<std::unique_lock<std::shared_mutex>> lockFuture;
    std::optional<std::unique_lock<std::shared_mutex>> lock;

    void EnsureLocked()
    {
        if (!lock)
            lock = lockFuture.get();

        Assert_True(lock.has_value());
    }
};

} // namespace Engine
