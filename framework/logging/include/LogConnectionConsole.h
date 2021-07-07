/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/logging/LogConnectionBase.h>
#include <string>

namespace Belle2 {

  /** Implements a log connection to an IO Stream.
   *
   * Inherits from the abstract base class LogConnectionBase.
   */
  class LogConnectionConsole final: public LogConnectionBase {
  public:
    /** Constructor
     * @param outputFD The output file descriptor to write to
     * @param color whether color should be used for output
     */
    LogConnectionConsole(int outputFD, bool color);

    /** Constructor which automatically enables color if the file descriptor is a terminal and supports colors
     * @param outputFD The output file descriptor to write to.
     */
    explicit LogConnectionConsole(int outputFD): LogConnectionConsole(outputFD, terminalSupportsColors(outputFD)) {}

    /** Returns true if the given file descriptor is a tty and supports colors. */
    static bool terminalSupportsColors(int fileDescriptor);

    /** Destructor */
    ~LogConnectionConsole() override;

    /** Sends a log message.
     * @param message The log message object.
     * @return true if the message could be send.
     */
    bool sendMessage(const LogMessage& message) override;

    /** Send a preformatted string message to the connected output */
    void write(const std::string& message);

    /** Returns true if the connection to the io stream could be established. */
    bool isConnected() override;

    /** Check whether console logging via python is enabled */
    static bool getPythonLoggingEnabled() { return s_pythonLoggingEnabled; }
    /** Set whether console logging via pyhthon is enabled */
    static void setPythonLoggingEnabled(bool enabled) { s_pythonLoggingEnabled = enabled; }
    /** Check whether we want to escape newlines on console */
    static bool getEscapeNewlinesEnabled() { return s_escapeNewlinesEnabled; }
    /** Set whether we want to escape newlines on console */
    static void setEscapeNewlinesEnabled(bool enabled) { s_escapeNewlinesEnabled = enabled; }
    /** Make sure output is flushed on abort */
    void finalizeOnAbort() override;
  private:
    int m_fd;  /**< The output stream used for sending the log message.*/
    bool m_color; /**< Flag for color output.*/
    static bool s_pythonLoggingEnabled; /**< Flag to indicate whether log messages should be sent to python sys.stdout */
    static bool s_escapeNewlinesEnabled; /**< Flag to indicate whether newlines should be replaced by '\n' in the output */
  };

} // end namespace Belle2
