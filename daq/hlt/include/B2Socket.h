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
#include <unistd.h>
#include <string>
#include <iostream>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>

#include <framework/logging/Logger.h>
#include <daq/hlt/NodeInfo.h>
#include <daq/hlt/HLTDefs.h>

#define MAXPACKETSIZE 1024000

namespace Belle2 {

  //! B2Socket class
  /*! This class is a base class for raw socket data communication
  */
  class B2Socket {
  public:
    //! Constructor
    B2Socket();
    B2Socket(int maxhosts, int maxcons, int maxrecv);
    //! Destructor
    virtual ~B2Socket();

    //! Creating socket
    EStatus create();
    //! Binding to specific port the socket created
    EStatus bind(const int port);
    //! Listening from the socket
    EStatus listen() const;
    //! Accepting a socket request
    int accept(B2Socket&) const;

    //! Connecting function to a specific host through specific port
    EStatus connect(const std::string host, const int port);

    ///! Sending data as a string format
    EStatus send(const std::string) const;
    //! Sending node information as NodeInfo object
    EStatus send(NodeInfo*) const;
    //! Receiving data as a string format
    int recv(std::string&) const;
    //! Receiving node information as NodeInfo object
    int recv(NodeInfo&) const;

    //! Setting non-blocking status
    void set_non_blocking(const bool);

    //! Validation checking
    EStatus is_valid() const;

    //void displayError(const std::string caller, int errno);

  private:
    int m_sock;               /*!< Socket identifier */
    sockaddr_in m_addr;       /*!< sockaddr_in structure for manipulating socket */

    int m_maxhosts;           /*!< Maximum number of hosts allowed to connect */
    int m_maxcons;            /*!< Maximum number of connection allowed */
    int m_maxrecv;            /*!< Maximum number of receiving allowed */
  };
}

#endif
