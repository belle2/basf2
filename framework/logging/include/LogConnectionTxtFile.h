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

#include <framework/logging/LogConnectionBase.h>

#include <string>
#include <iosfwd>

namespace Belle2 {

  /**
   * Implements a log connection to a text file.
   *
   * Inherits from the abstract base class LogConnectionBase.
  */
  class LogConnectionTxtFile : public LogConnectionBase {

  public:

    /**
     * The constructor.
     *
     * @param filename The path and name of the text file.
     * @param append If this value is set to true, an existing file will not be overridden,
     *               but instead the log messages will be added to the end of the file.
     */
    LogConnectionTxtFile(const std::string& filename, bool append = false);

    /** The LogConnectionTxtFile destructor. */
    virtual ~LogConnectionTxtFile();

    /**
     * Sends a log message.
     *
     * @param message The log message object.
     * @return Returns true if the message could be send.
     */
    virtual bool sendMessage(const LogMessage& message);

    /**
     * Returns true if the connection to the text file could be established.
     *
     * @return True if the connection to the text file could be established.
     */
    virtual bool isConnected();

  protected:

  private:

    std::ofstream* m_fileStream; /**< The file output stream used for sending the log message.*/

  };

} // end namespace Belle2
