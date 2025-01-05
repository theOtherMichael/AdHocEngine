#include <Engine/Console.h>

#include <gtest/gtest.h>

#include <string>

namespace Console = Engine::Console;
using Console::LogLevel;

class ConsoleTest : public testing::Test
{
protected:
    std::string defaultLogStreamOutput;
    std::string errorLogStreamOutput;
    std::string warningLogStreamOutput;
    std::string messageLogStreamOutput;
    std::string traceLogStreamOutput;

    Console::LogStream defaultLogStream = {[this](std::string logMessage) { defaultLogStreamOutput = logMessage; }};
    Console::LogStream errorLogStream   = {
        LogLevel::Error, [this](std::string logMessage) { errorLogStreamOutput = logMessage; }};
    Console::LogStream warningLogStream = {
        LogLevel::Warning, [this](std::string logMessage) { warningLogStreamOutput = logMessage; }};
    Console::LogStream messageLogStream = {
        LogLevel::Message, [this](std::string logMessage) { messageLogStreamOutput = logMessage; }};
    Console::LogStream traceLogStream = {
        LogLevel::Trace, [this](std::string logMessage) { traceLogStreamOutput = logMessage; }};
};

TEST_F(ConsoleTest, LogStreamFiltersErrorsCorrectly)
{
    Console::LogError("Test");

    EXPECT_EQ(defaultLogStreamOutput, "Test");
    EXPECT_EQ(errorLogStreamOutput, "Test");
    EXPECT_EQ(warningLogStreamOutput, "Test");
    EXPECT_EQ(messageLogStreamOutput, "Test");
    EXPECT_EQ(traceLogStreamOutput, "Test");
}

TEST_F(ConsoleTest, LogStreamFiltersWarningsCorrectly)
{
    Console::LogWarning("Test");

    EXPECT_EQ(defaultLogStreamOutput, "Test");
    EXPECT_EQ(errorLogStreamOutput, std::string());
    EXPECT_EQ(warningLogStreamOutput, "Test");
    EXPECT_EQ(messageLogStreamOutput, "Test");
    EXPECT_EQ(traceLogStreamOutput, "Test");
}

TEST_F(ConsoleTest, LogStreamFiltersMessagesCorrectly)
{
    Console::Log("Test");

    EXPECT_EQ(defaultLogStreamOutput, "Test");
    EXPECT_EQ(errorLogStreamOutput, std::string());
    EXPECT_EQ(warningLogStreamOutput, std::string());
    EXPECT_EQ(messageLogStreamOutput, "Test");
    EXPECT_EQ(traceLogStreamOutput, "Test");
}

TEST_F(ConsoleTest, LogStreamFiltersTracesCorrectly)
{
    Console::LogTrace("Test");

    EXPECT_EQ(defaultLogStreamOutput, std::string());
    EXPECT_EQ(errorLogStreamOutput, std::string());
    EXPECT_EQ(warningLogStreamOutput, std::string());
    EXPECT_EQ(messageLogStreamOutput, std::string());
    EXPECT_EQ(traceLogStreamOutput, "Test");
}

TEST_F(ConsoleTest, FmtStringsWork)
{
    Console::LogError("{1} {0} {2}", 1, 'b', "see");

    EXPECT_EQ(defaultLogStreamOutput, "b 1 see");
    EXPECT_EQ(errorLogStreamOutput, "b 1 see");
    EXPECT_EQ(warningLogStreamOutput, "b 1 see");
    EXPECT_EQ(messageLogStreamOutput, "b 1 see");
    EXPECT_EQ(traceLogStreamOutput, "b 1 see");
}
