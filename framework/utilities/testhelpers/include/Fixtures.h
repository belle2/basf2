/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/logging/LogSystem.h>
#include <gtest/gtest.h>

namespace Belle2::TestHelpers {
  /** Test fixture to be able to check the contents and types of emitted log
   * messages in detail
   *
   * In contrast to EXPECT_B2ERROR() and others defined here this test fixture
   * will keep a copy of every message emitted and all messages can be
   * expected in detail. There are a few convenient members to check for
   * message counts and message contents.
   */
  class LogMessageTest: public ::testing::Test {
  protected:
    /** list of log messages */
    std::vector<LogMessage> m_messages;
    /** Add a log message interceptor */
    void SetUp() override;
    /** And try to reset logging system to default */
    void TearDown() override;
    /** check if there is a certain number of log messages of a given level
     * @param level which log level to check for
     * @param count how many message should be expected
     * @param strict if true don't allow any other messages: the total number of
     *     messages must be equal to count and all need to have severity level
     */
    void expectMessage(LogConfig::ELogLevel level = LogConfig::c_Error, int count = 1, bool strict = false);
    /** check the content of the last log message against the given values
     * @param level log level we expect for the message
     * @param message expected string content of the message.
     */
    void expectMessageContent(LogConfig::ELogLevel level, const std::string& message);
    /** check if the last message emitted contains the given variables and their values.
     * @param variables map of name->value mapping for all expected variables
     * @param strict if true also fail the test if there's any other variable in the message
     */
    void expectMessageVariables(std::map<std::string, std::string>&& variables, bool strict = false);
    /** Expect an error with variables: This is a shorthand to check that there
     * is exactly one error message present and that it contains the given
     * variables. This also clears the list of messages for convenience so after
     * each call which is supposed to create an error this function can be
     * called to handle the error message and afterwards the list of messages
     * will be clean again for the next test.
     *
     * This is a shorthand to just call expectMessage(), expectMessageVariables() and clearMessages()
     *
     * @param variables map of name->value mapping for all expected variables
     * @param strict if true also fail the test if there's any other variable in the message
     */
    void expectErrorWithVariables(std::map<std::string, std::string>&& variables, bool strict = false);
    /** clear the list of save log messages */
    void clearMessages()
    {
      m_messages.clear();
    }
  };
} // namespace Belle2::TestHelpers
