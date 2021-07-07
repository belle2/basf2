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
#include <fstream>

namespace Belle2 {

  /**
   * Implements a log connection to a text file.
   *
   * Inherits from the abstract base class LogConnectionBase.
  */
  class LogConnectionTxtFile final: public LogConnectionBase {

  public:

    /**
     * The constructor.
     *
     * @param filename The path and name of the text file.
     * @param append If this value is set to true, an existing file will not be overridden,
     *               but instead the log messages will be added to the end of the file.
     */
    explicit LogConnectionTxtFile(const std::string& filename, bool append = false);

    /** The LogConnectionTxtFile destructor. */
    virtual ~LogConnectionTxtFile();

    /**
     * Sends a log message.
     *
     * @param message The log message object.
     * @return Returns true if the message could be send.
     */
    bool sendMessage(const LogMessage& message) override;

    /**
     * Returns true if the connection to the text file could be established.
     *
     * @return True if the connection to the text file could be established.
     */
    bool isConnected() override;

    /** Make sure the file is closed on abort */
    void finalizeOnAbort() override;

  private:
    std::ofstream m_fileStream; /**< The file output stream used for sending the log message.*/

  };

} // end namespace Belle2
