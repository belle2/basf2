/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/logging/LogConnectionConsole.h>

namespace Belle2 {

  /**
   * Implements a log connection to stdout but with messages formatted as json
   * objects to allow easy parsing by other tools, say jupyter notebooks.
   *
   * Inherits from the abstract base class LogConnectionBase and uses
   * LogConnectionConsole to do the actual output
  */
  class LogConnectionJSON final: public LogConnectionBase {

  public:

    /** The constructor.
     * @param complete if true ignore the LogInfo settings and output all parts of the message
     * (like file, line, etc.)
     */
    explicit LogConnectionJSON(bool complete = false);

    /** The LogConnectionJSONFile destructor. */
    ~LogConnectionJSON() override = default;

    /**
     * Sends a log message.
     *
     * @param message The log message object.
     * @return Returns true if the message could be send.
     */
    bool sendMessage(const LogMessage& message) override;

    /**
     * Returns true if the connection to the text file could be established.
     */
    bool isConnected() override { return m_console.isConnected(); }

    /** Finalize on Abort: make sure that connected connection is finalized */
    void finalizeOnAbort() override
    {
      m_console.finalizeOnAbort();
    }

  private:
    /** LogConnection to the console we use to dispatch the json objects */
    LogConnectionConsole m_console;
    /** If true ignore the log message format settings and output everything */
    bool m_complete;
  };

} // end namespace Belle2
