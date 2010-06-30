/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef LOGCONNECTIONIOSTREAM_H_
#define LOGCONNECTIONIOSTREAM_H_

#include <logging/LogCommon.h>
#include <logging/LogConnectionAbs.h>

#include <string>
#include <iostream>

namespace Belle2 {

  //! Implements a log connection to a IO Stream.
  /*!
     Inherits from the abstract base class LogConnectionAbs.
  */
  class LogConnectionIOStream : public LogConnectionAbs {

  public:

    //! The constructor
    /*!
      \param outputStream The output stream.
    */
    LogConnectionIOStream(std::ostream& outputStream);

    //! The LogConnectionIOStream destructor
    virtual ~LogConnectionIOStream();

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

    //! Returns true if the connection to the io stream could be established.
    /*!
     \return True if the connection to the io stream could be established.
    */
    virtual bool isConnected();

  protected:

  private:

    std::ostream* m_stream;  /*!< The output stream used for sending the log message.*/

  };

} // end namespace Belle2

#endif /* LOGCONNECTIONIOSTREAM_H_ */
