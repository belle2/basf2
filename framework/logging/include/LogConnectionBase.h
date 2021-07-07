/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

namespace Belle2 {
  class LogMessage;

  /**
   * Abstract base class for the different types of log connections.
   */
  class LogConnectionBase {

  public:

    /** The LogConnectionBase default constructor. */
    LogConnectionBase() {}

    /** The LogConnectionBase destructor. */
    virtual ~LogConnectionBase() {}

    /**
     * Sends a log message.
     *
     * @param message The log message object.
     * @return Returns true if the message could be send.
     */
    virtual bool sendMessage(const LogMessage& message) = 0;

    /**
     * Returns true if the connection could be established.
     *
     * @return True if the connection could be established.
    */
    virtual bool isConnected() = 0;

    /** Called just before the system exists after a fatal error. Can be used
     * to flush the output or close the file */
    virtual void finalizeOnAbort() {}
  };

} // end namespace Belle2
