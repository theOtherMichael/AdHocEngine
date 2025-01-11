#include <Engine/Core/Console.h>

#include <gtest/gtest.h>

#include <sstream>
#include <string>

namespace Console = Engine::Console;
using Console::LogLevel;

namespace Core
{

class ConsoleTest : public testing::Test
{
protected:
    std::string defaultStreamReceivedMessage;
    std::string fatalStreamReceivedMessage;
    std::string errorStreamReceivedMessage;
    std::string warningStreamReceivedMessage;
    std::string basicStreamReceivedMessage;
    std::string traceStreamReceivedMessage;

    LogLevel defaultStreamReceivedLogLevel;
    LogLevel fatalStreamReceivedLogLevel;
    LogLevel errorStreamReceivedLogLevel;
    LogLevel warningStreamReceivedLogLevel;
    LogLevel basicStreamReceivedLogLevel;
    LogLevel traceStreamReceivedLogLevel;

    Console::LogStream defaultLogStream = {[this](LogLevel logLevel, std::string logMessage)
                                           {
                                               defaultStreamReceivedLogLevel = logLevel;
                                               defaultStreamReceivedMessage  = logMessage;
                                           }};

    Console::LogStream fatalLogStream   = {LogLevel::Fatal,
                                           [this](LogLevel logLevel, std::string logMessage)
                                           {
                                             fatalStreamReceivedLogLevel = logLevel;
                                             fatalStreamReceivedMessage  = logMessage;
                                         }};
    Console::LogStream errorLogStream   = {LogLevel::Error,
                                           [this](LogLevel logLevel, std::string logMessage)
                                           {
                                             errorStreamReceivedLogLevel = logLevel;
                                             errorStreamReceivedMessage  = logMessage;
                                         }};
    Console::LogStream warningLogStream = {LogLevel::Warning,
                                           [this](LogLevel logLevel, std::string logMessage)
                                           {
                                               warningStreamReceivedLogLevel = logLevel;
                                               warningStreamReceivedMessage  = logMessage;
                                           }};
    Console::LogStream messageLogStream = {LogLevel::Log,
                                           [this](LogLevel logLevel, std::string logMessage)
                                           {
                                               basicStreamReceivedLogLevel = logLevel;
                                               basicStreamReceivedMessage  = logMessage;
                                           }};
    Console::LogStream traceLogStream   = {LogLevel::Trace,
                                           [this](LogLevel logLevel, std::string logMessage)
                                           {
                                             traceStreamReceivedLogLevel = logLevel;
                                             traceStreamReceivedMessage  = logMessage;
                                         }};
};

TEST_F(ConsoleTest, LogStreamsReceiveFatalLogsCorrectly)
{
    Console::Internal::LogImplementation(Console::LogLevel::Fatal, "Test");

    EXPECT_EQ(fatalStreamReceivedMessage, "Test");
    EXPECT_EQ(errorStreamReceivedMessage, "Test");
    EXPECT_EQ(warningStreamReceivedMessage, "Test");
    EXPECT_EQ(defaultStreamReceivedMessage, "Test");
    EXPECT_EQ(basicStreamReceivedMessage, "Test");
    EXPECT_EQ(traceStreamReceivedMessage, "Test");
}

TEST_F(ConsoleTest, LogStreamsReceiveErrorLogsCorrectly)
{
    Console::LogError("Test");

    EXPECT_EQ(fatalStreamReceivedMessage, "");
    EXPECT_EQ(errorStreamReceivedMessage, "Test");
    EXPECT_EQ(warningStreamReceivedMessage, "Test");
    EXPECT_EQ(defaultStreamReceivedMessage, "Test");
    EXPECT_EQ(basicStreamReceivedMessage, "Test");
    EXPECT_EQ(traceStreamReceivedMessage, "Test");
}

TEST_F(ConsoleTest, LogStreamsReceiveWarningLogsCorrectly)
{
    Console::LogWarning("Test");

    EXPECT_EQ(fatalStreamReceivedMessage, "");
    EXPECT_EQ(errorStreamReceivedMessage, "");
    EXPECT_EQ(warningStreamReceivedMessage, "Test");
    EXPECT_EQ(defaultStreamReceivedMessage, "Test");
    EXPECT_EQ(basicStreamReceivedMessage, "Test");
    EXPECT_EQ(traceStreamReceivedMessage, "Test");
}

TEST_F(ConsoleTest, LogStreamsReceiveBasicLogsCorrectly)
{
    Console::Log("Test");

    EXPECT_EQ(fatalStreamReceivedMessage, "");
    EXPECT_EQ(errorStreamReceivedMessage, "");
    EXPECT_EQ(warningStreamReceivedMessage, "");
    EXPECT_EQ(defaultStreamReceivedMessage, "Test");
    EXPECT_EQ(basicStreamReceivedMessage, "Test");
    EXPECT_EQ(traceStreamReceivedMessage, "Test");
}

TEST_F(ConsoleTest, LogStreamsReceiveTraceLogsCorrectly)
{
    Console::LogTrace("Test");

    EXPECT_EQ(fatalStreamReceivedMessage, "");
    EXPECT_EQ(errorStreamReceivedMessage, "");
    EXPECT_EQ(warningStreamReceivedMessage, "");
    EXPECT_EQ(defaultStreamReceivedMessage, "");
    EXPECT_EQ(basicStreamReceivedMessage, "");
    EXPECT_EQ(traceStreamReceivedMessage, "Test");
}

TEST_F(ConsoleTest, FmtStringsWork)
{
    Console::Internal::LogImplementation(LogLevel::Fatal, fmt::format("{1} {0} {2}", 1, 'b', "see"));

    EXPECT_EQ(fatalStreamReceivedMessage, "b 1 see");
    EXPECT_EQ(errorStreamReceivedMessage, "b 1 see");
    EXPECT_EQ(warningStreamReceivedMessage, "b 1 see");
    EXPECT_EQ(defaultStreamReceivedMessage, "b 1 see");
    EXPECT_EQ(basicStreamReceivedMessage, "b 1 see");
    EXPECT_EQ(traceStreamReceivedMessage, "b 1 see");
}

TEST_F(ConsoleTest, LogStreamsReceiveLogLevels)
{
    Console::Internal::LogImplementation(LogLevel::Fatal, "Test");

    EXPECT_EQ(fatalStreamReceivedLogLevel, LogLevel::Fatal);
    EXPECT_EQ(errorStreamReceivedLogLevel, LogLevel::Fatal);
    EXPECT_EQ(warningStreamReceivedLogLevel, LogLevel::Fatal);
    EXPECT_EQ(defaultStreamReceivedLogLevel, LogLevel::Fatal);
    EXPECT_EQ(basicStreamReceivedLogLevel, LogLevel::Fatal);
    EXPECT_EQ(traceStreamReceivedLogLevel, LogLevel::Fatal);

    Console::LogError("Test");

    EXPECT_EQ(fatalStreamReceivedLogLevel, LogLevel::Fatal);
    EXPECT_EQ(errorStreamReceivedLogLevel, LogLevel::Error);
    EXPECT_EQ(warningStreamReceivedLogLevel, LogLevel::Error);
    EXPECT_EQ(defaultStreamReceivedLogLevel, LogLevel::Error);
    EXPECT_EQ(basicStreamReceivedLogLevel, LogLevel::Error);
    EXPECT_EQ(traceStreamReceivedLogLevel, LogLevel::Error);

    Console::LogWarning("Test");

    EXPECT_EQ(fatalStreamReceivedLogLevel, LogLevel::Fatal);
    EXPECT_EQ(errorStreamReceivedLogLevel, LogLevel::Error);
    EXPECT_EQ(warningStreamReceivedLogLevel, LogLevel::Warning);
    EXPECT_EQ(defaultStreamReceivedLogLevel, LogLevel::Warning);
    EXPECT_EQ(basicStreamReceivedLogLevel, LogLevel::Warning);
    EXPECT_EQ(traceStreamReceivedLogLevel, LogLevel::Warning);

    Console::Log("Test");

    EXPECT_EQ(fatalStreamReceivedLogLevel, LogLevel::Fatal);
    EXPECT_EQ(errorStreamReceivedLogLevel, LogLevel::Error);
    EXPECT_EQ(warningStreamReceivedLogLevel, LogLevel::Warning);
    EXPECT_EQ(defaultStreamReceivedLogLevel, LogLevel::Log);
    EXPECT_EQ(basicStreamReceivedLogLevel, LogLevel::Log);
    EXPECT_EQ(traceStreamReceivedLogLevel, LogLevel::Log);

    Console::LogTrace("Test");

    EXPECT_EQ(fatalStreamReceivedLogLevel, LogLevel::Fatal);
    EXPECT_EQ(errorStreamReceivedLogLevel, LogLevel::Error);
    EXPECT_EQ(warningStreamReceivedLogLevel, LogLevel::Warning);
    EXPECT_EQ(defaultStreamReceivedLogLevel, LogLevel::Log);
    EXPECT_EQ(basicStreamReceivedLogLevel, LogLevel::Log);
    EXPECT_EQ(traceStreamReceivedLogLevel, LogLevel::Trace);
}

} // namespace Core
