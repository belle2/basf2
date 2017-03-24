/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <iostream>
#include <framework/utilities/IOIntercept.h>
#include <framework/logging/LogSystem.h>
#include <framework/logging/LogConnectionIOStream.h>
#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;

namespace {
  /** intercept log messages and store them in a given vector */
  class LogInterceptor final: public LogConnectionBase {
    /** reference to where they should be stored */
    std::vector<LogMessage>& m_messages;
  public:
    /** remember where to put the messages */
    LogInterceptor(std::vector<LogMessage>& messages): m_messages(messages) {}
    /** always connected */
    bool isConnected() { return true; }
    /** and always sucessful in storing into buffer */
    bool sendMessage(const LogMessage& message) { m_messages.emplace_back(message); return true; }
  };

  /** params for testing */
  typedef std::tuple<LogConfig::ELogLevel, std::string, std::string, bool, bool> logconvert_params_t;

  /** Fixture class to intercept log messages which is needed for some tests */
  class IOInterceptTest: public ::testing::Test {
  protected:
    /** list of log messages */
    std::vector<LogMessage> m_messages;

    /** Add a log message interceptor */
    void SetUp()
    {
      m_messages.clear();
      LogSystem::Instance().resetLogConnections();
      LogSystem::Instance().addLogConnection(new LogInterceptor(m_messages));
    }

    /** And try to reset logging system to default */
    void TearDown()
    {
      LogSystem::Instance().resetLogConnections();
      bool useColor = LogConnectionIOStream::terminalSupportsColors();
      LogSystem::Instance().addLogConnection(new LogConnectionIOStream(std::cout, useColor));
      LogSystem::Instance().resetMessageCounter();
    }
  };

  /** Typedef to allow the death test sharing the fixture */
  typedef IOInterceptTest IOInterceptDeathTest;

  /** Derive a parametrized fixture to test conversion to log messages */
  class IOInterceptParamTest: public IOInterceptTest, public ::testing::WithParamInterface<logconvert_params_t> {};

  /** Test if output is converted to a log message correctly */
  TEST_P(IOInterceptParamTest, ConvertToLogMessage)
  {
    // first let's get the parameters
    LogConfig::ELogLevel level;
    std::string raw_message, formatted_message;
    bool useStdout, generateMessage;
    std::tie(level, raw_message, formatted_message, useStdout, generateMessage) = GetParam();
    // now create some output and convert it to log messages
    IOIntercept::OutputToLogMessages capture("capture_name", level, level);
    capture.start();
    (useStdout ? std::cout : std::cerr) << raw_message;
    capture.finish();
    // check that the amount of log messages is correct
    ASSERT_EQ(m_messages.size(), generateMessage ? 1 : 0);
    if (!generateMessage) return;
    // and also check the message level and content
    LogMessage& msg = m_messages.front();
    ASSERT_EQ(msg.getLogLevel(), level);
    ASSERT_EQ(msg.getMessage(), "Output from capture_name:\ncapture_name: " + formatted_message);
  }

  /** all the parameter combinations we want to test for conversion
   * the fields are: log level, raw message, formatted message, use stdout?, should message be generated?
   */
  logconvert_params_t logconvert_params[] = {
    // test info message for stdout and stderr
    {LogConfig::c_Info, "foo" , "foo", false, true},
    {LogConfig::c_Info, "foo" , "foo", true, true},
    // also for error
    {LogConfig::c_Error, "foo" , "foo", false, true},
    {LogConfig::c_Error, "foo" , "foo", true, true},
    // ok, test the other levels only for stdout
    {LogConfig::c_Warning, "foo" , "foo", false, true},
    {LogConfig::c_Result, "foo" , "foo", false, true},
    // test that empty message gets removed
    {LogConfig::c_Info, "" , "", false, false},
    // also with whitespace
    {LogConfig::c_Info, " " , "", false, false},
    {LogConfig::c_Info, " \t " , "", false, false},
    {LogConfig::c_Info, " \n " , "", false, false},
    // trim space at end
    {LogConfig::c_Info, "message\n\n   \t\r\n" , "message", false, true},
    // trim space at beginning
    {LogConfig::c_Info, "   \n\t\nmessage" , "message", false, true},
    // but not the spaces in the first line which also contains text to not break alignment of formatted output
    {LogConfig::c_Info, "   \n\tmessage" , "\tmessage", false, true},
    {LogConfig::c_Info, "   \n  message" , "  message", false, true},

  };

  /** instantiate tests for all the parameters */
  INSTANTIATE_TEST_CASE_P(Params, IOInterceptParamTest, ::testing::ValuesIn(logconvert_params));

  /** check that the indentation is applied for all lines */
  TEST_F(IOInterceptTest, ConvertCheckIndent)
  {
    IOIntercept::OutputToLogMessages capture("indent");
    capture.start();
    std::cout << "this is\na multi line message";
    capture.finish();
    ASSERT_EQ(m_messages.size(), 1);
    LogMessage& msg = m_messages.front();
    ASSERT_EQ(msg.getMessage(), "Output from indent:\nindent: this is\nindent: a multi line message");
  }

  /** and that it can be set correctly */
  TEST_F(IOInterceptTest, ConvertSetIndent)
  {
    IOIntercept::OutputToLogMessages capture("indent");
    capture.start();
    std::cout << "this is\na multi line message";
    capture.setIndent("--->");
    capture.finish();
    ASSERT_EQ(m_messages.size(), 1);
    LogMessage& msg = m_messages.front();
    ASSERT_EQ(msg.getMessage(), "Output from indent:\n--->this is\n--->a multi line message");
  }

  /** empty indentation should work too */
  TEST_F(IOInterceptTest, ConvertEmptyIndent)
  {
    IOIntercept::OutputToLogMessages capture("indent");
    capture.start();
    capture.setIndent("");
    std::cout << "this is\na multi line message";
    capture.finish();
    ASSERT_EQ(m_messages.size(), 1);
    LogMessage& msg = m_messages.front();
    ASSERT_EQ(msg.getMessage(), "Output from indent:\nthis is\na multi line message");
  }

  /** test that capturing stdout works */
  TEST_F(IOInterceptTest, CaptureStdOut)
  {
    IOIntercept::CaptureStdOut capture;
    ASSERT_FALSE(capture.finish());
    ASSERT_TRUE(capture.start());
    ASSERT_TRUE(capture.start());
    std::cout << "this is a test";
    ASSERT_TRUE(capture.finish());
    ASSERT_EQ(capture.getStdOut(), "this is a test");
    ASSERT_FALSE(capture.finish());
    ASSERT_EQ(capture.getStdOut(), "this is a test");

    // Ok, again with raw write to fd
    ASSERT_TRUE(capture.start());
    write(fileno(stdout), "this is a test", 14);
    ASSERT_TRUE(capture.finish());
    ASSERT_EQ(capture.getStdOut(), "this is a test");
  }

  /** test that capturing stderr works */
  TEST_F(IOInterceptTest, CaptureStdErr)
  {
    IOIntercept::CaptureStdErr capture;
    ASSERT_FALSE(capture.finish());
    ASSERT_TRUE(capture.start());
    ASSERT_TRUE(capture.start());
    std::cerr << "this is a test";
    ASSERT_TRUE(capture.finish());
    ASSERT_EQ(capture.getStdErr(), "this is a test");
    ASSERT_FALSE(capture.finish());
    ASSERT_EQ(capture.getStdErr(), "this is a test");

    // Ok, again with raw write to fd
    ASSERT_TRUE(capture.start());
    write(fileno(stderr), "this is a test", 14);
    ASSERT_TRUE(capture.finish());
    ASSERT_EQ(capture.getStdErr(), "this is a test");

  }

  /** test if capturing large output works as expected */
  TEST_F(IOInterceptTest, CaptureLargeOutput)
  {
    IOIntercept::CaptureStdOut capture;
    ASSERT_TRUE(capture.start());
    std::string out;
    int written{0};
    for (int i = 0; i < 100000; ++i) {
      std::cout << (char)(i % 255) << flush;
      if (std::cout.good()) written = i + 1;
      out.push_back(i % 255);
    }
    ASSERT_TRUE(capture.finish());
    const std::string& captured = capture.getStdOut();
    // pipes have a limited capacity. We create them non-blocking which
    // means that once they're full we will just not get any output back.
    // So check that the begin of the capture works
    ASSERT_TRUE(captured.size() <= out.size());
    ASSERT_EQ(captured.size(), written);
    if (captured.size() < out.size()) {
      std::cout << "Output truncated after " << captured.size() << " bytes" << std::endl;
    }
    ASSERT_EQ(captured, out.substr(0, captured.size()));
    // and that capturing still works after overflow
    ASSERT_TRUE(capture.start());
    std::cout << "once more";
    ASSERT_TRUE(capture.finish());
    ASSERT_EQ(capture.getStdOut(), "once more");

    // and once more for C
    ASSERT_TRUE(capture.start());
    printf("once more");
    ASSERT_TRUE(capture.finish());
    ASSERT_EQ(capture.getStdOut(), "once more");
  }

  /** this function generates a bit of output on stderr and exits. If
   * discarding output works the output is "start-><-end", otherwise there
   * will be additional text between the arrows */
  void generateStdErr()
  {
    IOIntercept::DiscardStdErr discard;
    std::cerr << "start->";
    discard.start();
    std::cerr << "this should not show up" << std::endl << std::flush;
    write(fileno(stderr), "nor this\n", 9);
    discard.finish();
    std::cerr << "<-end";
    std::exit(0);
  }

  /** test discarding output */
  TEST_F(IOInterceptDeathTest, DiscardStdOut)
  {
    IOIntercept::DiscardStdOutStdErr discard;
    ASSERT_FALSE(discard.finish());
    ASSERT_TRUE(discard.start());
    ASSERT_TRUE(discard.start());
    ASSERT_TRUE(discard.finish());
    ASSERT_FALSE(discard.finish());
    // hard to test if there's no output ... let's use a death test and
    // verify that stderr of child process matches what we expect
    EXPECT_EXIT(generateStdErr(), ::testing::ExitedWithCode(0), "^start-><-end$");
  }
}
