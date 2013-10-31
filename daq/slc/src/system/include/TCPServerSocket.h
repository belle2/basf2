#ifndef _B2ARICH_TCPServerSocket_hh_
#define _B2ARICH_TCPServerSocket_hh_

#include "TCPSocket.h"
#include "FileDescriptor.h"

namespace Belle2 {

  class TCPServerSocket : public FileDescriptor {

  public:
    TCPServerSocket()
      : _ip(""), _port(0) {}
    TCPServerSocket(const std::string& ip, unsigned short port)
      : _ip(ip), _port(port) {}
    virtual ~TCPServerSocket() throw() {}

  public:
    void setIP(const std::string& ip) throw() { _ip = ip; }
    void setPort(unsigned short port) throw() { _port = port; }
    const std::string& getIP() const throw() { return _ip; }
    unsigned short getPort() const throw() { return _port; }
    int open() throw(IOException);
    int open(std::string& ip, unsigned short port) throw(IOException);
    TCPSocket accept() throw(IOException);

  private:
    std::string _ip;
    unsigned short _port;

  };

}

#endif

