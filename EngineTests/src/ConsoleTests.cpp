#include <Engine/Console.h>

#include <gtest/gtest.h>

#include <sstream>
#include <string>

namespace Console = Engine::Console;
using Console::LogLevel;

class ConsoleTest : public testing::Test
{
protected:
    std::string defaultStreamReceivedMessage;
    std::string errorStreamReceivedMessage;
    std::string warningStreamReceivedMessage;
    std::string messageStreamReceivedMessage;
    std::string traceStreamReceivedMessage;

    LogLevel defaultStreamReceivedLogLevel;
    LogLevel errorStreamReceivedLogLevel;
    LogLevel warningStreamReceivedLogLevel;
    LogLevel messageStreamReceivedLogLevel;
    LogLevel traceStreamReceivedLogLevel;

    Console::LogStream defaultLogStream = {[this](LogLevel logLevel, std::string logMessage)
                                           {
                                               defaultStreamReceivedLogLevel = logLevel;
                                               defaultStreamReceivedMessage  = logMessage;
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
    Console::LogStream messageLogStream = {LogLevel::Message,
                                           [this](LogLevel logLevel, std::string logMessage)
                                           {
                                               messageStreamReceivedLogLevel = logLevel;
                                               messageStreamReceivedMessage  = logMessage;
                                           }};
    Console::LogStream traceLogStream   = {LogLevel::Trace,
                                           [this](LogLevel logLevel, std::string logMessage)
                                           {
                                             traceStreamReceivedLogLevel = logLevel;
                                             traceStreamReceivedMessage  = logMessage;
                                         }};
};

TEST_F(ConsoleTest, LogStreamFiltersErrorsCorrectly)
{
    Console::LogError("Test");

    EXPECT_EQ(defaultStreamReceivedMessage, "Test");
    EXPECT_EQ(errorStreamReceivedMessage, "Test");
    EXPECT_EQ(warningStreamReceivedMessage, "Test");
    EXPECT_EQ(messageStreamReceivedMessage, "Test");
    EXPECT_EQ(traceStreamReceivedMessage, "Test");
}

TEST_F(ConsoleTest, LogStreamFiltersWarningsCorrectly)
{
    Console::LogWarning("Test");

    EXPECT_EQ(defaultStreamReceivedMessage, "Test");
    EXPECT_EQ(errorStreamReceivedMessage, std::string());
    EXPECT_EQ(warningStreamReceivedMessage, "Test");
    EXPECT_EQ(messageStreamReceivedMessage, "Test");
    EXPECT_EQ(traceStreamReceivedMessage, "Test");
}

TEST_F(ConsoleTest, LogStreamFiltersMessagesCorrectly)
{
    Console::Log("Test");

    EXPECT_EQ(defaultStreamReceivedMessage, "Test");
    EXPECT_EQ(errorStreamReceivedMessage, std::string());
    EXPECT_EQ(warningStreamReceivedMessage, std::string());
    EXPECT_EQ(messageStreamReceivedMessage, "Test");
    EXPECT_EQ(traceStreamReceivedMessage, "Test");
}

TEST_F(ConsoleTest, LogStreamFiltersTracesCorrectly)
{
    Console::LogTrace("Test");

    EXPECT_EQ(defaultStreamReceivedMessage, std::string());
    EXPECT_EQ(errorStreamReceivedMessage, std::string());
    EXPECT_EQ(warningStreamReceivedMessage, std::string());
    EXPECT_EQ(messageStreamReceivedMessage, std::string());
    EXPECT_EQ(traceStreamReceivedMessage, "Test");
}

TEST_F(ConsoleTest, FmtStringsWork)
{
    Console::LogError("{1} {0} {2}", 1, 'b', "see");

    EXPECT_EQ(defaultStreamReceivedMessage, "b 1 see");
    EXPECT_EQ(errorStreamReceivedMessage, "b 1 see");
    EXPECT_EQ(warningStreamReceivedMessage, "b 1 see");
    EXPECT_EQ(messageStreamReceivedMessage, "b 1 see");
    EXPECT_EQ(traceStreamReceivedMessage, "b 1 see");
}

TEST_F(ConsoleTest, LogLevelsOstreamable)
{
    std::ostringstream sstream;
    sstream << LogLevel::Error << LogLevel::Warning << LogLevel::Message << LogLevel::Trace;
    ASSERT_EQ(sstream.str(), "ErrorWarningMessageTrace");
}

TEST_F(ConsoleTest, LogStreamsReceiveLogLevel)
{
    Console::LogError("Test");

    EXPECT_EQ(defaultStreamReceivedLogLevel, LogLevel::Error);
    EXPECT_EQ(errorStreamReceivedLogLevel, LogLevel::Error);
    EXPECT_EQ(warningStreamReceivedLogLevel, LogLevel::Error);
    EXPECT_EQ(messageStreamReceivedLogLevel, LogLevel::Error);
    EXPECT_EQ(traceStreamReceivedLogLevel, LogLevel::Error);
}
