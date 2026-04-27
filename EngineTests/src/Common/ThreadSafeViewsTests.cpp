#include <Engine/Common/ThreadSafeViews.h>

#include <gtest/gtest.h>

#include <chrono>
#include <shared_mutex>
#include <thread>

using namespace std::chrono_literals;

namespace Common
{

struct TestData
{
    int value;
};

TEST(ThreadSafeViewsTest, ViewAccessorsWork)
{
    auto mutex = std::shared_mutex{};
    auto data  = TestData{42};

    {
        auto sharedView = Engine::ThreadSafeSharedViewAsync(mutex, data);
        EXPECT_EQ(sharedView->value, 42);
        EXPECT_EQ((*sharedView).value, 42);
    }

    {
        auto exclusiveView = Engine::ThreadSafeExclusiveViewAsync(mutex, data);
        EXPECT_EQ(exclusiveView->value, 42);
        EXPECT_EQ((*exclusiveView).value, 42);
    }
}

TEST(ThreadSafeViewsTest, SharedViewAccessorsDontBlock)
{
    auto mutex = std::shared_mutex{};
    auto data  = TestData{42};

    const auto startTime = std::chrono::system_clock::now();

    const auto reader1 = std::jthread(
        [&]()
        {
            auto sharedView = Engine::ThreadSafeSharedViewAsync(mutex, data);
            EXPECT_EQ(sharedView->value, 42);
            std::this_thread::sleep_for(100ms);
        });
    const auto reader2 = std::jthread(
        [&]()
        {
            auto sharedView = Engine::ThreadSafeSharedViewAsync(mutex, data);
            EXPECT_EQ(sharedView->value, 42);
            std::this_thread::sleep_for(100ms);
        });
    const auto reader3 = std::jthread(
        [&]()
        {
            auto sharedView = Engine::ThreadSafeSharedViewAsync(mutex, data);
            EXPECT_EQ(sharedView->value, 42);
            std::this_thread::sleep_for(100ms);
        });

    const auto endTime = std::chrono::system_clock::now();

    ASSERT_LT(endTime - startTime, 300ms);
}

TEST(ThreadSafeViewsTest, ExclusiveViewsBlockEachOther)
{
    auto mutex = std::shared_mutex{};
    auto data  = TestData{42};

    const auto startTime = std::chrono::system_clock::now();

    auto writer1 = std::thread(
        [&]()
        {
            auto exclusiveView   = Engine::ThreadSafeExclusiveViewAsync(mutex, data);
            exclusiveView->value = 100;
            EXPECT_EQ(exclusiveView->value, 100);
            std::this_thread::sleep_for(100ms);
        });
    auto writer2 = std::thread(
        [&]()
        {
            auto exclusiveView   = Engine::ThreadSafeExclusiveViewAsync(mutex, data);
            exclusiveView->value = 200;
            EXPECT_EQ(exclusiveView->value, 200);
            std::this_thread::sleep_for(100ms);
        });
    auto writer3 = std::thread(
        [&]()
        {
            auto exclusiveView   = Engine::ThreadSafeExclusiveViewAsync(mutex, data);
            exclusiveView->value = 300;
            EXPECT_EQ(exclusiveView->value, 300);
            std::this_thread::sleep_for(100ms);
        });

    writer1.join();
    writer2.join();
    writer3.join();

    const auto endTime = std::chrono::system_clock::now();

    ASSERT_GT(endTime - startTime, 300ms);
}

TEST(ThreadSafeViewsTest, ExclusiveViewsBlockSharedViews)
{
    auto mutex = std::shared_mutex{};
    auto data  = TestData{42};

    const auto writer = std::jthread(
        [&]()
        {
            auto exclusiveView   = Engine::ThreadSafeExclusiveViewAsync(mutex, data);
            exclusiveView->value = 100;
            std::this_thread::sleep_for(100ms);
        });

    std::this_thread::sleep_for(10ms); // Ensure writer starts first

    const auto reader = std::jthread(
        [&]()
        {
            auto sharedView = Engine::ThreadSafeSharedViewAsync(mutex, data);
            EXPECT_EQ(sharedView->value, 100);
        });
}

TEST(ThreadSafeViewsTest, SharedViewsLockAsynchronously)
{
    auto mutex = std::shared_mutex{};
    auto data  = TestData{42};

    const auto startTime = std::chrono::system_clock::now();

    const auto writer = std::jthread(
        [&]()
        {
            auto exclusiveView   = Engine::ThreadSafeExclusiveViewAsync(mutex, data);
            exclusiveView->value = 100;
            std::this_thread::sleep_for(300ms);
        });

    std::this_thread::sleep_for(10ms); // Ensure writer thread has started

    auto sharedView = Engine::ThreadSafeSharedViewAsync(mutex, data);
    EXPECT_FALSE(sharedView.IsReady());

    const auto firstEndTime = std::chrono::system_clock::now();
    EXPECT_LT(firstEndTime - startTime, 300ms);

    EXPECT_EQ(sharedView->value, 100);
    const auto secondEndTime = std::chrono::system_clock::now();
    EXPECT_GT(secondEndTime - startTime, 300ms);

    EXPECT_TRUE(sharedView.IsReady());
}

TEST(ThreadSafeViewsTest, ExclusiveViewsLockAsynchronously)
{
    auto mutex = std::shared_mutex{};
    auto data  = TestData{42};

    const auto startTime = std::chrono::system_clock::now();

    const auto reader1 = std::jthread(
        [&]()
        {
            Engine::ThreadSafeSharedViewAsync<TestData> sharedView(mutex, data);
            EXPECT_EQ(sharedView->value, 42);
            std::this_thread::sleep_for(300ms);
        });
    const auto reader2 = std::jthread(
        [&]()
        {
            Engine::ThreadSafeSharedViewAsync<TestData> sharedView(mutex, data);
            EXPECT_EQ(sharedView->value, 42);
            std::this_thread::sleep_for(300ms);
        });
    const auto reader3 = std::jthread(
        [&]()
        {
            Engine::ThreadSafeSharedViewAsync<TestData> sharedView(mutex, data);
            EXPECT_EQ(sharedView->value, 42);
            std::this_thread::sleep_for(300ms);
        });

    std::this_thread::sleep_for(10ms); // Ensure reader threads have started

    auto exclusiveView = Engine::ThreadSafeExclusiveViewAsync(mutex, data);
    EXPECT_FALSE(exclusiveView.IsReady());

    const auto firstEndTime = std::chrono::system_clock::now();
    EXPECT_LT(firstEndTime - startTime, 300ms);

    EXPECT_EQ(exclusiveView->value, 42);
    const auto secondEndTime = std::chrono::system_clock::now();
    EXPECT_GT(secondEndTime - startTime, 300ms);

    EXPECT_TRUE(exclusiveView.IsReady());
}

} // namespace Common
