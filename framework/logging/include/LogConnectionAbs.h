/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef LOGCONNECTIONABS_H_
#define LOGCONNECTIONABS_H_

#include <logging/LogCommon.h>

#include <string>

namespace Belle2 {

  //! Abstract base class for the different types of log connections.
  /*!
  */
  class LogConnectionAbs {

  public:

    LogConnectionAbs() {};

    virtual ~LogConnectionAbs() {};

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
                             const std::string& function, const std::string& file, unsigned int line, bool sendLocationInfo) = 0;

    //! Returns true if the connection could be established.
    /*!
     \return True if the connection could be established.
    */
    virtual bool isConnected() = 0;

  protected:

  private:

  };

} // end namespace Belle2

#endif /* LOGCONNECTIONABS_H_ */
