#include <asl/finally.h>

#include <gtest/gtest.h>

#include <utility>

// Shamelessly adapted from https://github.com/microsoft/GSL/ (MIT License)

namespace
{

void f(int& i)
{
    i += 1;
}

static int j = 0;

void g()
{
    j += 1;
}

} // namespace

namespace asl
{

TEST(FinallyTests, FinallyLambda)
{
    int i = 0;
    {
        auto _ = asl::finally([&]() { f(i); });
        EXPECT_TRUE(i == 0);
    }
    EXPECT_TRUE(i == 1);
}

TEST(FinallyTests, FinallyLambdaMove)
{
    int i = 0;
    {
        auto _1 = asl::finally([&]() { f(i); });
        {
            auto _2 = std::move(_1);
            EXPECT_TRUE(i == 0);
        }
        EXPECT_TRUE(i == 1);
        {
            auto _2 = std::move(_1); // NOLINT(bugprone-use-after-move,clang-analyzer-cplusplus.Move) -- intentional: verifies move-from-moved is a no-op
            EXPECT_TRUE(i == 1);
        }
        EXPECT_TRUE(i == 1);
    }
    EXPECT_TRUE(i == 1);
}

TEST(FinallyTests, FinallyConstLvalueLambda)
{
    int i = 0;
    {
        const auto const_lvalue_lambda = [&]() { f(i); };
        auto _                         = asl::finally(const_lvalue_lambda);
        EXPECT_TRUE(i == 0);
    }
    EXPECT_TRUE(i == 1);
}

TEST(FinallyTests, FinallyMutableLvalueLambda)
{
    int i = 0;
    {
        auto mutable_lvalue_lambda = [&]() { f(i); };
        auto _                     = asl::finally(mutable_lvalue_lambda);
        EXPECT_TRUE(i == 0);
    }
    EXPECT_TRUE(i == 1);
}

TEST(FinallyTests, FinallyFunctionWithBind)
{
    int i = 0;
    {
        auto _ = asl::finally([&i] { return f(i); });
        EXPECT_TRUE(i == 0);
    }
    EXPECT_TRUE(i == 1);
}

TEST(FinallyTests, FinallyFunctionPtr)
{
    j = 0;
    {
        auto _ = asl::finally(&g);
        EXPECT_TRUE(j == 0);
    }
    EXPECT_TRUE(j == 1);
}

TEST(FinallyTests, FinallyFunction)
{
    j = 0;
    {
        auto _ = asl::finally(g);
        EXPECT_TRUE(j == 0);
    }
    EXPECT_TRUE(j == 1);
}

} // namespace asl
