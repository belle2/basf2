/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef B2SOCKET_H
#define B2SOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <framework/logging/Logger.h>
#include <daq/hlt/HLTDefs.h>

namespace Belle2 {

  //! B2Socket class
  /*! This class is a base class for data communication using raw socket
  */
  class B2Socket {
  public:
    //! Constructor
    B2Socket();
    //! Destructor
    ~B2Socket();

    //! Create socket
    EHLTStatus create();
    //! Bind to specific port the socket created
    EHLTStatus bind(const unsigned int port);
    //! Make a connection
    EHLTStatus connect(const std::string destination, const int port);
    //! Listen to the socket
    EHLTStatus listen();
    //! Accept request from other node via socket
    EHLTStatus accept(int& newSocket);
    //! Send data through socket
    EHLTStatus send(const std::string data, int& size);
    EHLTStatus send(char* data, int size);
    //! Receive data from socket
    EHLTStatus receive(int newSocket, std::string& buffer, int& size);
    EHLTStatus receive(int newSocket, char* buffer, int& size);

  protected:
    //! Check if the socket is valid or not
    bool isValid();
    //! Set socket as non-blocking mode
    void setNonBlocking(const bool flag);

  private:
    int m_socket;                   /**< Socket identifier */
    sockaddr_in m_socketAddress;    /**< sockaddr_in structure to manipulate the socket */

    int m_maxHosts;                 /**< Maximum number of hosts allowed to connect */
    int m_maxConnections;           /**< Maximum number of connections allowed */
    int m_maxReceives;              /**< Maximum size of received data allowed */
  };
}

#endif
