/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
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

  protected:

  private:

  };

} // end namespace Belle2
