#include <Engine/Core/Assertions.h>

#include <gtest/gtest.h>

#pragma clang diagnostic ignored "-Wunused-value"
#pragma clang diagnostic ignored "-Wunused-comparison"

// These sometimes erroneously trigger a crash for some reason
#undef DEBUG_BREAK
#define DEBUG_BREAK()

namespace Console = Engine::Console;

namespace Core
{

TEST(AssertionTest, PassingAssertsDontTriggerCrash)
{
    Assert_NoReentry();

    Assert_True(true);
    Assert_False(false);
    Assert_Eq(1, 1);
    Assert_Ne(1, 0);
    Assert_Lt(1, 2);
    Assert_Le(1, 1);
    Assert_Le(1, 2);
    Assert_Gt(2, 1);
    Assert_Ge(2, 2);
    Assert_Ge(2, 1);

    AssertEval_True(true);
    AssertEval_False(false);
    AssertEval_Eq(1, 1);
    AssertEval_Ne(1, 0);
    AssertEval_Lt(1, 2);
    AssertEval_Le(1, 1);
    AssertEval_Le(1, 2);
    AssertEval_Gt(2, 1);
    AssertEval_Ge(2, 2);
    AssertEval_Ge(2, 1);

    Assert_True_Slow(true);
    Assert_False_Slow(false);
    Assert_Eq_Slow(1, 1);
    Assert_Ne_Slow(1, 0);
    Assert_Lt_Slow(1, 2);
    Assert_Le_Slow(1, 1);
    Assert_Le_Slow(1, 2);
    Assert_Gt_Slow(2, 1);
    Assert_Ge_Slow(2, 2);
    Assert_Ge_Slow(2, 1);

    AssertEval_True_Slow(true);
    AssertEval_False_Slow(false);
    AssertEval_Eq_Slow(1, 1);
    AssertEval_Ne_Slow(1, 0);
    AssertEval_Lt_Slow(1, 2);
    AssertEval_Le_Slow(1, 1);
    AssertEval_Le_Slow(1, 2);
    AssertEval_Gt_Slow(2, 1);
    AssertEval_Ge_Slow(2, 2);
    AssertEval_Ge_Slow(2, 1);

    SUCCEED();
}

#if ADHOC_ASSERTIONS_ON
TEST(AssertionDeathTest, FailingAssertsTriggerCrash)
{
    EXPECT_DEATH(Assert_NoEntry(), "");
    EXPECT_DEATH(
        for (auto i = 0; i < 2; i++) { Assert_NoReentry(); }, "");

    EXPECT_DEATH(Assert_True(false), "");
    EXPECT_DEATH(Assert_False(true), "");
    EXPECT_DEATH(Assert_Eq(1, 0), "");
    EXPECT_DEATH(Assert_Ne(1, 1), "");
    EXPECT_DEATH(Assert_Lt(2, 1), "");
    EXPECT_DEATH(Assert_Le(2, 1), "");
    EXPECT_DEATH(Assert_Gt(1, 2), "");
    EXPECT_DEATH(Assert_Ge(1, 2), "");

    EXPECT_DEATH(AssertEval_True(false), "");
    EXPECT_DEATH(AssertEval_False(true), "");
    EXPECT_DEATH(AssertEval_Eq(1, 0), "");
    EXPECT_DEATH(AssertEval_Ne(1, 1), "");
    EXPECT_DEATH(AssertEval_Lt(2, 1), "");
    EXPECT_DEATH(AssertEval_Le(2, 1), "");
    EXPECT_DEATH(AssertEval_Gt(1, 2), "");
    EXPECT_DEATH(AssertEval_Ge(1, 2), "");

    #if ADHOC_SLOW_ASSERTIONS_ON
    EXPECT_DEATH(Assert_True_Slow(false), "");
    EXPECT_DEATH(Assert_False_Slow(true), "");
    EXPECT_DEATH(Assert_Eq_Slow(1, 0), "");
    EXPECT_DEATH(Assert_Ne_Slow(1, 1), "");
    EXPECT_DEATH(Assert_Lt_Slow(2, 1), "");
    EXPECT_DEATH(Assert_Le_Slow(2, 1), "");
    EXPECT_DEATH(Assert_Gt_Slow(1, 2), "");
    EXPECT_DEATH(Assert_Ge_Slow(1, 2), "");

    EXPECT_DEATH(AssertEval_True_Slow(false), "");
    EXPECT_DEATH(AssertEval_False_Slow(true), "");
    EXPECT_DEATH(AssertEval_Eq_Slow(1, 0), "");
    EXPECT_DEATH(AssertEval_Ne_Slow(1, 1), "");
    EXPECT_DEATH(AssertEval_Lt_Slow(2, 1), "");
    EXPECT_DEATH(AssertEval_Le_Slow(2, 1), "");
    EXPECT_DEATH(AssertEval_Gt_Slow(1, 2), "");
    EXPECT_DEATH(AssertEval_Ge_Slow(1, 2), "");
    #endif // ADHOC_SLOW_ASSERTIONS_ON
}
#endif // ADHOC_ASSERTIONS_ON

TEST(AssertionTest, PassingExpectsDontLog)
{
    auto lastErrorLog = std::string();
    auto errorStream  = Console::LogStream(Console::LogLevel::Error,
                                          [&lastErrorLog](const Console::LogLevel logLevel, const std::string& message)
                                          { lastErrorLog = message; });

#define PASSING_EXPECTS_DONT_LOG_CASE(assertion)                                                                       \
    {                                                                                                                  \
        assertion;                                                                                                     \
        EXPECT_EQ(lastErrorLog, "");                                                                                   \
    }

    PASSING_EXPECTS_DONT_LOG_CASE(Expect_NoReentry());

    PASSING_EXPECTS_DONT_LOG_CASE(Expect_True(true));
    PASSING_EXPECTS_DONT_LOG_CASE(Expect_False(false));
    PASSING_EXPECTS_DONT_LOG_CASE(Expect_Eq(1, 1));
    PASSING_EXPECTS_DONT_LOG_CASE(Expect_Ne(1, 0));
    PASSING_EXPECTS_DONT_LOG_CASE(Expect_Lt(1, 2));
    PASSING_EXPECTS_DONT_LOG_CASE(Expect_Le(1, 1));
    PASSING_EXPECTS_DONT_LOG_CASE(Expect_Le(1, 2));
    PASSING_EXPECTS_DONT_LOG_CASE(Expect_Gt(2, 1));
    PASSING_EXPECTS_DONT_LOG_CASE(Expect_Ge(2, 2));
    PASSING_EXPECTS_DONT_LOG_CASE(Expect_Ge(2, 1));

    PASSING_EXPECTS_DONT_LOG_CASE(ExpectEval_True(true));
    PASSING_EXPECTS_DONT_LOG_CASE(ExpectEval_False(false));
    PASSING_EXPECTS_DONT_LOG_CASE(ExpectEval_Eq(1, 1));
    PASSING_EXPECTS_DONT_LOG_CASE(ExpectEval_Ne(1, 0));
    PASSING_EXPECTS_DONT_LOG_CASE(ExpectEval_Lt(1, 2));
    PASSING_EXPECTS_DONT_LOG_CASE(ExpectEval_Le(1, 1));
    PASSING_EXPECTS_DONT_LOG_CASE(ExpectEval_Le(1, 2));
    PASSING_EXPECTS_DONT_LOG_CASE(ExpectEval_Gt(2, 1));
    PASSING_EXPECTS_DONT_LOG_CASE(ExpectEval_Ge(2, 2));
    PASSING_EXPECTS_DONT_LOG_CASE(ExpectEval_Ge(2, 1));

    PASSING_EXPECTS_DONT_LOG_CASE(Expect_True_Slow(true));
    PASSING_EXPECTS_DONT_LOG_CASE(Expect_False_Slow(false));
    PASSING_EXPECTS_DONT_LOG_CASE(Expect_Eq_Slow(1, 1));
    PASSING_EXPECTS_DONT_LOG_CASE(Expect_Ne_Slow(1, 0));
    PASSING_EXPECTS_DONT_LOG_CASE(Expect_Lt_Slow(1, 2));
    PASSING_EXPECTS_DONT_LOG_CASE(Expect_Le_Slow(1, 1));
    PASSING_EXPECTS_DONT_LOG_CASE(Expect_Le_Slow(1, 2));
    PASSING_EXPECTS_DONT_LOG_CASE(Expect_Gt_Slow(2, 1));
    PASSING_EXPECTS_DONT_LOG_CASE(Expect_Ge_Slow(2, 2));
    PASSING_EXPECTS_DONT_LOG_CASE(Expect_Ge_Slow(2, 1));

    PASSING_EXPECTS_DONT_LOG_CASE(ExpectEval_True_Slow(true));
    PASSING_EXPECTS_DONT_LOG_CASE(ExpectEval_False_Slow(false));
    PASSING_EXPECTS_DONT_LOG_CASE(ExpectEval_Eq_Slow(1, 1));
    PASSING_EXPECTS_DONT_LOG_CASE(ExpectEval_Ne_Slow(1, 0));
    PASSING_EXPECTS_DONT_LOG_CASE(ExpectEval_Lt_Slow(1, 2));
    PASSING_EXPECTS_DONT_LOG_CASE(ExpectEval_Le_Slow(1, 1));
    PASSING_EXPECTS_DONT_LOG_CASE(ExpectEval_Le_Slow(1, 2));
    PASSING_EXPECTS_DONT_LOG_CASE(ExpectEval_Gt_Slow(2, 1));
    PASSING_EXPECTS_DONT_LOG_CASE(ExpectEval_Ge_Slow(2, 2));
    PASSING_EXPECTS_DONT_LOG_CASE(ExpectEval_Ge_Slow(2, 1));

#undef PASSING_EXPECTS_DONT_LOG_CASE
}

#if ADHOC_ASSERTIONS_ON
TEST(AssertionTest, FailingExpectsLogErrors)
{
    auto lastErrorLog = std::string();
    auto errorStream  = Console::LogStream(Console::LogLevel::Error,
                                          [&lastErrorLog](const Console::LogLevel logLevel, const std::string& message)
                                          { lastErrorLog = message; });

    #define FAILING_EXPECTS_LOG_ERRORS_CASE(assertion)                                                                 \
        {                                                                                                              \
            assertion;                                                                                                 \
            EXPECT_NE(lastErrorLog, "");                                                                               \
            lastErrorLog.clear();                                                                                      \
        }

    FAILING_EXPECTS_LOG_ERRORS_CASE(Expect_NoEntry());
    FAILING_EXPECTS_LOG_ERRORS_CASE(for (int i = 0; i < 2; i++) Expect_NoReentry());

    FAILING_EXPECTS_LOG_ERRORS_CASE(Expect_True(false));
    FAILING_EXPECTS_LOG_ERRORS_CASE(Expect_False(true));
    FAILING_EXPECTS_LOG_ERRORS_CASE(Expect_Eq(1, 0));
    FAILING_EXPECTS_LOG_ERRORS_CASE(Expect_Ne(1, 1));
    FAILING_EXPECTS_LOG_ERRORS_CASE(Expect_Lt(2, 1));
    FAILING_EXPECTS_LOG_ERRORS_CASE(Expect_Le(2, 1));
    FAILING_EXPECTS_LOG_ERRORS_CASE(Expect_Gt(1, 2));
    FAILING_EXPECTS_LOG_ERRORS_CASE(Expect_Ge(1, 2));

    FAILING_EXPECTS_LOG_ERRORS_CASE(ExpectEval_True(false));
    FAILING_EXPECTS_LOG_ERRORS_CASE(ExpectEval_False(true));
    FAILING_EXPECTS_LOG_ERRORS_CASE(ExpectEval_Eq(1, 0));
    FAILING_EXPECTS_LOG_ERRORS_CASE(ExpectEval_Ne(1, 1));
    FAILING_EXPECTS_LOG_ERRORS_CASE(ExpectEval_Lt(2, 1));
    FAILING_EXPECTS_LOG_ERRORS_CASE(ExpectEval_Le(2, 1));
    FAILING_EXPECTS_LOG_ERRORS_CASE(ExpectEval_Gt(1, 2));
    FAILING_EXPECTS_LOG_ERRORS_CASE(ExpectEval_Ge(1, 2));

    #if ADHOC_SLOW_ASSERTIONS_ON
    FAILING_EXPECTS_LOG_ERRORS_CASE(Expect_True_Slow(false));
    FAILING_EXPECTS_LOG_ERRORS_CASE(Expect_False_Slow(true));
    FAILING_EXPECTS_LOG_ERRORS_CASE(Expect_Eq_Slow(1, 0));
    FAILING_EXPECTS_LOG_ERRORS_CASE(Expect_Ne_Slow(1, 1));
    FAILING_EXPECTS_LOG_ERRORS_CASE(Expect_Lt_Slow(2, 1));
    FAILING_EXPECTS_LOG_ERRORS_CASE(Expect_Le_Slow(2, 1));
    FAILING_EXPECTS_LOG_ERRORS_CASE(Expect_Gt_Slow(1, 2));
    FAILING_EXPECTS_LOG_ERRORS_CASE(Expect_Ge_Slow(1, 2));

    FAILING_EXPECTS_LOG_ERRORS_CASE(ExpectEval_True_Slow(false));
    FAILING_EXPECTS_LOG_ERRORS_CASE(ExpectEval_False_Slow(true));
    FAILING_EXPECTS_LOG_ERRORS_CASE(ExpectEval_Eq_Slow(1, 0));
    FAILING_EXPECTS_LOG_ERRORS_CASE(ExpectEval_Ne_Slow(1, 1));
    FAILING_EXPECTS_LOG_ERRORS_CASE(ExpectEval_Lt_Slow(2, 1));
    FAILING_EXPECTS_LOG_ERRORS_CASE(ExpectEval_Le_Slow(2, 1));
    FAILING_EXPECTS_LOG_ERRORS_CASE(ExpectEval_Gt_Slow(1, 2));
    FAILING_EXPECTS_LOG_ERRORS_CASE(ExpectEval_Ge_Slow(1, 2));
    #endif // ADHOC_SLOW_ASSERTIONS_ON

    #undef FAILING_EXPECTS_LOG_ERRORS_CASE
}
#endif // ADHOC_ASSERTIONS_ON

#if ADHOC_ASSERTIONS_ON
TEST(AssertionTest, ExpressionsEvaluateOnlyOnce)
{
    auto evaluationCount = 0;

    #define EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(assertion)                                                             \
        {                                                                                                              \
            {assertion};                                                                                               \
            EXPECT_EQ(evaluationCount, 1);                                                                             \
            evaluationCount = 0;                                                                                       \
        }

    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(Expect_True(++evaluationCount == 1));
    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(Expect_False(++evaluationCount == 0));

    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(Expect_Eq(++evaluationCount, 1));
    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(Expect_Ne(++evaluationCount, 0));
    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(Expect_Lt(++evaluationCount, 2));
    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(Expect_Le(++evaluationCount, 1));
    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(Expect_Gt(++evaluationCount, 0));
    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(Expect_Ge(++evaluationCount, 1));

    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(ExpectEval_True(++evaluationCount == 1));
    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(ExpectEval_False(++evaluationCount == 0));

    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(ExpectEval_Eq(++evaluationCount, 1));
    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(ExpectEval_Ne(++evaluationCount, 0));
    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(ExpectEval_Lt(++evaluationCount, 2));
    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(ExpectEval_Le(++evaluationCount, 1));
    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(ExpectEval_Gt(++evaluationCount, 0));
    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(ExpectEval_Ge(++evaluationCount, 1));

    #if ADHOC_SLOW_ASSERTIONS_ON
    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(Expect_True_Slow(++evaluationCount == 1));
    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(Expect_False_Slow(++evaluationCount == 0));

    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(Expect_Eq_Slow(++evaluationCount, 1));
    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(Expect_Ne_Slow(++evaluationCount, 0));
    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(Expect_Lt_Slow(++evaluationCount, 2));
    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(Expect_Le_Slow(++evaluationCount, 1));
    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(Expect_Gt_Slow(++evaluationCount, 0));
    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(Expect_Ge_Slow(++evaluationCount, 1));

    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(ExpectEval_True_Slow(++evaluationCount == 1));
    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(ExpectEval_False_Slow(++evaluationCount == 0));

    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(ExpectEval_Eq_Slow(++evaluationCount, 1));
    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(ExpectEval_Ne_Slow(++evaluationCount, 0));
    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(ExpectEval_Lt_Slow(++evaluationCount, 2));
    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(ExpectEval_Le_Slow(++evaluationCount, 1));
    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(ExpectEval_Gt_Slow(++evaluationCount, 0));
    EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE(ExpectEval_Ge_Slow(++evaluationCount, 1));
    #endif

    #undef EXPRESSIONS_EVALUATE_ONLY_ONCE_CASE
}
#endif // ADHOC_ASSERTIONS_ON

#if !ADHOC_SLOW_ASSERTIONS_ON
TEST(AssertionTest, NonEvalAssertionsDontEvalWhenOff)
{
    auto someVariable = 0;

    #define NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(assertion)                                                          \
        assertion;                                                                                                     \
        EXPECT_EQ(someVariable, 0);                                                                                    \
        someVariable = 0;

    #if !ADHOC_ASSERTIONS_ON
    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Assert_True(++someVariable == 1));
    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Assert_False(++someVariable == 0));
    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Assert_Eq(++someVariable, 1));
    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Assert_Ne(++someVariable, 0));
    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Assert_Lt(++someVariable, 2));
    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Assert_Le(++someVariable, 2));
    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Assert_Gt(++someVariable, 0));
    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Assert_Ge(++someVariable, 0));

    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Expect_True(++someVariable == 1));
    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Expect_False(++someVariable == 0));
    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Expect_Eq(++someVariable, 1));
    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Expect_Ne(++someVariable, 0));
    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Expect_Lt(++someVariable, 2));
    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Expect_Le(++someVariable, 2));
    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Expect_Gt(++someVariable, 0));
    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Expect_Ge(++someVariable, 0));
    #endif // !ADHOC_ASSERTIONS_ON

    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Assert_True_Slow(++someVariable == 1));
    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Assert_False_Slow(++someVariable == 0));
    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Assert_Eq_Slow(++someVariable, 1));
    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Assert_Ne_Slow(++someVariable, 0));
    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Assert_Lt_Slow(++someVariable, 2));
    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Assert_Le_Slow(++someVariable, 2));
    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Assert_Gt_Slow(++someVariable, 0));
    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Assert_Ge_Slow(++someVariable, 0));

    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Expect_True_Slow(++someVariable == 1));
    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Expect_False_Slow(++someVariable == 0));
    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Expect_Eq_Slow(++someVariable, 1));
    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Expect_Ne_Slow(++someVariable, 0));
    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Expect_Lt_Slow(++someVariable, 2));
    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Expect_Le_Slow(++someVariable, 2));
    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Expect_Gt_Slow(++someVariable, 0));
    NON_EVAL_ASSERTIONS_DONT_EVAL_WHEN_OFF(Expect_Ge_Slow(++someVariable, 0));
}
#endif // !ADHOC_ASSERTIONS_ON

#if !ADHOC_SLOW_ASSERTIONS_ON
TEST(AssertionTest, EvalAssertionExpressionsEvaluateWhenOff)
{
    auto someVariable = 0;

    #define EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(assertion)                                               \
        assertion;                                                                                                     \
        EXPECT_EQ(someVariable, 1);                                                                                    \
        someVariable = 0;

    #if !ADHOC_ASSERTIONS_ON
    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(AssertEval_True(++someVariable == 1));
    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(AssertEval_False(++someVariable == 0));
    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(AssertEval_Eq(++someVariable, 1));
    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(AssertEval_Ne(++someVariable, 0));
    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(AssertEval_Lt(++someVariable, 2));
    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(AssertEval_Le(++someVariable, 2));
    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(AssertEval_Gt(++someVariable, 0));
    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(AssertEval_Ge(++someVariable, 0));

    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(ExpectEval_True(++someVariable == 1));
    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(ExpectEval_False(++someVariable == 0));
    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(ExpectEval_Eq(++someVariable, 1));
    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(ExpectEval_Ne(++someVariable, 0));
    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(ExpectEval_Lt(++someVariable, 2));
    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(ExpectEval_Le(++someVariable, 2));
    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(ExpectEval_Gt(++someVariable, 0));
    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(ExpectEval_Ge(++someVariable, 0));
    #endif // !ADHOC_ASSERTIONS_ON

    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(AssertEval_True_Slow(++someVariable == 1));
    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(AssertEval_False_Slow(++someVariable == 0));
    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(AssertEval_Eq_Slow(++someVariable, 1));
    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(AssertEval_Ne_Slow(++someVariable, 0));
    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(AssertEval_Lt_Slow(++someVariable, 2));
    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(AssertEval_Le_Slow(++someVariable, 2));
    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(AssertEval_Gt_Slow(++someVariable, 0));
    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(AssertEval_Ge_Slow(++someVariable, 0));

    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(ExpectEval_True_Slow(++someVariable == 1));
    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(ExpectEval_False_Slow(++someVariable == 0));
    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(ExpectEval_Eq_Slow(++someVariable, 1));
    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(ExpectEval_Ne_Slow(++someVariable, 0));
    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(ExpectEval_Lt_Slow(++someVariable, 2));
    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(ExpectEval_Le_Slow(++someVariable, 2));
    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(ExpectEval_Gt_Slow(++someVariable, 0));
    EVAL_ASSERTION_EXPRESSIONS_EVALUATE_WHEN_OFF_CASE(ExpectEval_Ge_Slow(++someVariable, 0));
}
#endif // !ADHOC_ASSERTIONS_ON

TEST(AssertionTest, AssertCodeBlocksWork)
{
    auto codeBlockHasRun = false;

    Assert_Code(codeBlockHasRun = true;);

#if ADHOC_ASSERTIONS_ON
    ASSERT_TRUE(codeBlockHasRun);
#else
    ASSERT_FALSE(codeBlockHasRun);
#endif

    codeBlockHasRun = false;

    Assert_Code_Slow(codeBlockHasRun = true;);

#if ADHOC_SLOW_ASSERTIONS_ON
    ASSERT_TRUE(codeBlockHasRun);
#else
    ASSERT_FALSE(codeBlockHasRun);
#endif
}

} // namespace Core
