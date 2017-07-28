/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2017 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/logging/LogConnectionBase.h>

namespace Belle2 {

  /** Implements a log connection to an IO Stream.
   *
   * Inherits from the abstract base class LogConnectionBase.
   */
  class LogConnectionFileDescriptor : public LogConnectionBase {

  public:


    /** Constructor
     * @param outputFD The output file descriptor to write to
     * @param color whether color should be used for output
     */
    LogConnectionFileDescriptor(int outputFD, bool color);

    /** Constructor which automatically enables color if the file descriptor is a terminal and supports colors
     * @param outputFD The output file descriptor to write to.
     */
    explicit LogConnectionFileDescriptor(int outputFD): LogConnectionFileDescriptor(outputFD, terminalSupportsColors(outputFD)) {}

    /** Returns true if the given file descriptor is a tty and supports colors. */
    static bool terminalSupportsColors(int fileDescriptor);

    /** Destructor */
    virtual ~LogConnectionFileDescriptor();

    /** Sends a log message.
     * @param message The log message object.
     * @return true if the message could be send.
     */
    virtual bool sendMessage(const LogMessage& message);

    /** Returns true if the connection to the io stream could be established. */
    virtual bool isConnected();

  private:
    int m_fd;  /**< The output stream used for sending the log message.*/
    bool m_color; /**< Flag for color output.*/
  };

} // end namespace Belle2
