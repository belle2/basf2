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

#include <framework/logging/LogConnectionBase.h>

#include <iosfwd>

namespace Belle2 {

  /**
   * Implements a log connection to an IO Stream.
   *
   * Inherits from the abstract base class LogConnectionBase.
   */
  class LogConnectionIOStream : public LogConnectionBase {

  public:

    /**
     * The constructor.
     *
     * @param outputStream The output stream.
     * @param color Wether to print messages of different log levels
     *              in different colors.
     */
    LogConnectionIOStream(std::ostream& outputStream, bool color = false);

    /**
     * The LogConnectionIOStream destructor.
     */
    virtual ~LogConnectionIOStream();

    /**
     * Sends a log message.
     *
     * @param message The log message object.
     * @return Returns true if the message could be send.
     */
    virtual bool sendMessage(const LogMessage& message);

    /**
     * Returns true if the connection to the io stream could be established.
     *
     * @return True if the connection to the io stream could be established.
     */
    virtual bool isConnected();

  protected:

  private:

    std::ostream& m_stream;  /**< The output stream used for sending the log message.*/
    bool m_color;            /**< Flag for color output.*/

  };

} // end namespace Belle2

#endif /* LOGCONNECTIONIOSTREAM_H_ */
