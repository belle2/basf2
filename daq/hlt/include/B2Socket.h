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
  class B2Socket {
  public:
    B2Socket();
    ~B2Socket();

    EHLTStatus create();
    EHLTStatus bind(const unsigned int port);
    EHLTStatus connect(const std::string destination, const int port);
    EHLTStatus listen();
    EHLTStatus accept(int& newSocket);
    EHLTStatus send(const std::string data, int& size);
    EHLTStatus receive(int newSocket, std::string& buffer, int& size);

  protected:
    bool isValid();
    void setNonBlocking(const bool flag);

  private:
    int m_socket;
    sockaddr_in m_socketAddress;

    int m_maxHosts, m_maxConnections, m_maxReceives;
  };
}

#endif
