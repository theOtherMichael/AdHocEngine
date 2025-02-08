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
    std::shared_mutex mutex;
    TestData data{42};

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
    std::shared_mutex mutex;
    TestData data{42};

    auto startTime = std::chrono::system_clock::now();

    std::thread reader1(
        [&]()
        {
            auto sharedView = Engine::ThreadSafeSharedViewAsync(mutex, data);
            EXPECT_EQ(sharedView->value, 42);
            std::this_thread::sleep_for(100ms);
        });
    std::thread reader2(
        [&]()
        {
            auto sharedView = Engine::ThreadSafeSharedViewAsync(mutex, data);
            EXPECT_EQ(sharedView->value, 42);
            std::this_thread::sleep_for(100ms);
        });
    std::thread reader3(
        [&]()
        {
            auto sharedView = Engine::ThreadSafeSharedViewAsync(mutex, data);
            EXPECT_EQ(sharedView->value, 42);
            std::this_thread::sleep_for(100ms);
        });

    reader1.join();
    reader2.join();
    reader3.join();
    auto endTime = std::chrono::system_clock::now();

    ASSERT_LT(endTime - startTime, 300ms);
}

TEST(ThreadSafeViewsTest, ExclusiveViewsBlockEachOther)
{
    std::shared_mutex mutex;
    TestData data{42};

    auto startTime = std::chrono::system_clock::now();

    std::thread writer1(
        [&]()
        {
            auto exclusiveView   = Engine::ThreadSafeExclusiveViewAsync(mutex, data);
            exclusiveView->value = 100;
            EXPECT_EQ(exclusiveView->value, 100);
            std::this_thread::sleep_for(100ms);
        });
    std::thread writer2(
        [&]()
        {
            auto exclusiveView   = Engine::ThreadSafeExclusiveViewAsync(mutex, data);
            exclusiveView->value = 200;
            EXPECT_EQ(exclusiveView->value, 200);
            std::this_thread::sleep_for(100ms);
        });
    std::thread writer3(
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
    auto endTime = std::chrono::system_clock::now();

    ASSERT_GT(endTime - startTime, 300ms);
}

TEST(ThreadSafeViewsTest, ExclusiveViewsBlockSharedViews)
{
    std::shared_mutex mutex;
    TestData data{42};

    std::thread writer(
        [&]()
        {
            auto exclusiveView   = Engine::ThreadSafeExclusiveViewAsync(mutex, data);
            exclusiveView->value = 100;
            std::this_thread::sleep_for(100ms);
        });

    std::this_thread::sleep_for(10ms); // Ensure writer starts first

    std::thread reader(
        [&]()
        {
            auto sharedView = Engine::ThreadSafeSharedViewAsync(mutex, data);
            EXPECT_EQ(sharedView->value, 100);
        });

    writer.join();
    reader.join();
}

TEST(ThreadSafeViewsTest, SharedViewsLockAsynchronously)
{
    std::shared_mutex mutex;
    TestData data{42};

    auto startTime = std::chrono::system_clock::now();

    std::thread writer(
        [&]()
        {
            auto exclusiveView   = Engine::ThreadSafeExclusiveViewAsync(mutex, data);
            exclusiveView->value = 100;
            std::this_thread::sleep_for(300ms);
        });

    std::this_thread::sleep_for(10ms); // Ensure writer thread has started

    auto sharedView = Engine::ThreadSafeSharedViewAsync(mutex, data);
    EXPECT_FALSE(sharedView.IsReady());

    auto endTime = std::chrono::system_clock::now();
    EXPECT_LT(endTime - startTime, 300ms);

    EXPECT_EQ(sharedView->value, 100);
    endTime = std::chrono::system_clock::now();
    EXPECT_GT(endTime - startTime, 300ms);

    EXPECT_TRUE(sharedView.IsReady());

    writer.join();
}

TEST(ThreadSafeViewsTest, ExclusiveViewsLockAsynchronously)
{
    std::shared_mutex mutex;
    TestData data{42};

    auto startTime = std::chrono::system_clock::now();

    std::thread reader1(
        [&]()
        {
            Engine::ThreadSafeSharedViewAsync<TestData> sharedView(mutex, data);
            EXPECT_EQ(sharedView->value, 42);
            std::this_thread::sleep_for(300ms);
        });
    std::thread reader2(
        [&]()
        {
            Engine::ThreadSafeSharedViewAsync<TestData> sharedView(mutex, data);
            EXPECT_EQ(sharedView->value, 42);
            std::this_thread::sleep_for(300ms);
        });
    std::thread reader3(
        [&]()
        {
            Engine::ThreadSafeSharedViewAsync<TestData> sharedView(mutex, data);
            EXPECT_EQ(sharedView->value, 42);
            std::this_thread::sleep_for(300ms);
        });

    std::this_thread::sleep_for(10ms); // Ensure reader threads have started

    auto exclusiveView = Engine::ThreadSafeExclusiveViewAsync(mutex, data);
    EXPECT_FALSE(exclusiveView.IsReady());

    auto endTime = std::chrono::system_clock::now();
    EXPECT_LT(endTime - startTime, 300ms);

    EXPECT_EQ(exclusiveView->value, 42);
    endTime = std::chrono::system_clock::now();
    EXPECT_GT(endTime - startTime, 300ms);

    EXPECT_TRUE(exclusiveView.IsReady());

    reader1.join();
    reader2.join();
    reader3.join();
}

} // namespace Common
