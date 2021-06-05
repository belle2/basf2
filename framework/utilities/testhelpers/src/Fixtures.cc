/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/utilities/testhelpers/Fixtures.h>
#include <framework/logging/LogConnectionBase.h>
#include <framework/logging/LogConnectionConsole.h>

#include <boost/algorithm/string.hpp>
#include <boost/range/adaptors.hpp>

namespace Belle2::TestHelpers {
  namespace {
    /** intercept log messages and store them in a given vector */
    class LogInterceptor final: public Belle2::LogConnectionBase {
      /** reference to where they should be stored */
      std::vector<Belle2::LogMessage>& m_messages;
    public:
      /** remember where to put the messages */
      explicit LogInterceptor(std::vector<Belle2::LogMessage>& messages): m_messages(messages) {}
      /** always connected */
      bool isConnected() override { return true; }
      /** and always sucessful in storing into buffer */
      bool sendMessage(Belle2::LogMessage const& message) override {m_messages.push_back(message); return false; }
    };
  }

  /** Add a log message interceptor */
  void LogMessageTest::SetUp()
  {
    m_messages.clear();
    LogSystem::Instance().resetLogConnections();
    LogSystem::Instance().addLogConnection(new LogInterceptor(m_messages));
    LogSystem::Instance().addLogConnection(new LogConnectionConsole(STDOUT_FILENO));
  }

  /** And try to reset logging system to default */
  void LogMessageTest::TearDown()
  {
    LogSystem::Instance().resetLogging();
  }

  void LogMessageTest::expectMessage(LogConfig::ELogLevel level, int count, bool strict)
  {
    int messages = std::count_if(m_messages.begin(), m_messages.end(), [level](const LogMessage & m) {
      return m.getLogLevel() == level;
    });
    EXPECT_EQ(count, messages) << "Expected " << count << " messages of loglevel " << LogConfig::logLevelToString(level)
                               << ", found " << messages;
    if (strict) {
      EXPECT_EQ(messages, m_messages.size());
    }
  }

  void LogMessageTest::expectMessageVariables(std::map<std::string, std::string>&& variables, bool strict)
  {
    ASSERT_FALSE(m_messages.empty()) << "No message available to check";
    const auto& msg = m_messages.back();
    for (const auto& var : msg.getLogVariables()) {
      if (auto it = variables.find(var.getName()); it != variables.end()) {
        EXPECT_EQ(it->second, var.getValue());
        variables.erase(it);
      } else {
        EXPECT_FALSE(strict) << "extra variable: " << var.getName() << std::endl;
      }
    }
    EXPECT_TRUE(variables.empty()) << "Some requested log variables were not present in the log message: "
                                   << boost::algorithm::join(variables | boost::adaptors::map_keys, ", ");
  }

  void LogMessageTest::expectErrorWithVariables(std::map<std::string, std::string>&& variables, bool strict)
  {
    expectMessage(LogConfig::c_Error, 1, true);
    expectMessageVariables(std::move(variables), strict);
    clearMessages();
  }

  void LogMessageTest::expectMessageContent(LogConfig::ELogLevel level, const std::string& message)
  {
    ASSERT_FALSE(m_messages.empty()) << "No message available to check";
    const auto& msg = m_messages.back();
    EXPECT_EQ(msg.getLogLevel(), level);
    EXPECT_EQ(msg.getMessage(), message);
  }
} // namespace Belle2::TestHelpers
