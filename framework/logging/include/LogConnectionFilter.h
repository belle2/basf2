/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/logging/LogConnectionBase.h>
#include <framework/logging/LogMessage.h>

#include <memory>

namespace Belle2 {

  /**
   * Implements a log connection that filters repeated messages.
   *
   * This connection can be used to prevent printing of the same message again
   * and again. Messages are suppressed if they match with the one previously sent.
   */
  class LogConnectionFilter final : public LogConnectionBase {

  public:

    /**
     * The constructor.
     *
     * @param logConnection The log connection that actually prints the messages.
     */
    explicit LogConnectionFilter(LogConnectionBase* logConnection);

    /**
     * The LogConnectionFilter destructor.
     */
    virtual ~LogConnectionFilter();

    /**
     * Sends a log message.
     *
     * @param message The log message object.
     * @return Returns true if the message could be send.
     */
    bool sendMessage(const LogMessage& message) override;

    /**
     * Returns true if the log connection could be established.
     *
     * @return True if the log connection could be established.
     */
    bool isConnected() override;

    /** Finalize on Abort: make sure last repetition is printed if there is any
     * and that connected connection is also finalized
     */
    void finalizeOnAbort() override
    {
      printRepetitions();
      m_logConnection->finalizeOnAbort();
    }

  private:
    /** If there are repeated messages print the number of repetitions.  */
    void printRepetitions();

    std::unique_ptr<LogConnectionBase> m_logConnection; /**< The actual log connection.*/
    LogMessage m_previousMessage;       /**< The previous log message. */
    int m_repeatCounter;                /**< Counter for repeated messages. */
  };

} // end namespace Belle2
