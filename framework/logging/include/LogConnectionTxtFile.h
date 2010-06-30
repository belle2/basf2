/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef LOGCONNECTIONTXTFILE_H_
#define LOGCONNECTIONTXTFILE_H_

#include <logging/LogCommon.h>
#include <logging/LogConnectionAbs.h>

#include <string>
#include <iostream>
#include <fstream>

namespace Belle2 {

  //! Implements a log connection to a text file.
  /*!
     Inherits from the abstract base class LogConnectionAbs.
  */
  class LogConnectionTxtFile : public LogConnectionAbs {

  public:

    //! The constructor
    /*!
      \param filename The path and name of the text file.
      \param append If this value is set to true, an existing file will not be overridden,
                    but instead the log messages will be added to the end of the file.
    */
    LogConnectionTxtFile(const std::string& filename, bool append = false);

    //! The LogConnectionTxtFile destructor
    virtual ~LogConnectionTxtFile();

    //! Sends a log message.
    /*!
     \param logLevel The log level of the message (e.g. debug, info, warning, error, fatal).
     \param message The message string which should be send.
     \param package The package name where the message was sent from.
     \param function The function name where the message was sent from.
     \param file The file name where the message was sent from.
     \param line The line number in the source code where the message was sent from.
     \param sendLocationInfo If true, the location info (package, function, file, line) is sent
     \return Returns true if the message could be send.
    */
    virtual bool sendMessage(LogCommon::ELogLevel logLevel, const std::string& message, const std::string& package,
                             const std::string& function, const std::string& file, unsigned int line, bool sendLocationInfo);

    //! Returns true if the connection to the text file could be established.
    /*!
     \return True if the connection to the text file could be established.
    */
    virtual bool isConnected();

  protected:

  private:

    std::ofstream* m_fileStream; /*!< The file output stream used for sending the log message.*/

  };

} // end namespace Belle2

#endif /* LOGCONNECTIONTXTFILE_H_ */
