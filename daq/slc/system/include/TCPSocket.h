#ifndef _B2ARICH_TCPSocket_hh_
#define _B2ARICH_TCPSocket_hh_

#include <string>

#include "daq/slc/system/FileDescriptor.h"

namespace Belle2 {

  class TCPSocket : public FileDescriptor {

    friend class TCPServerSocket;

  public:
    TCPSocket() : _ip(""), _port(0) {}
    TCPSocket(const std::string& ip, unsigned short port)
      : _ip(ip), _port(port) {}
    virtual ~TCPSocket() throw() {}

  private:
    TCPSocket(int fd) : FileDescriptor(fd), _ip(""), _port(0) {}

  public:
    void setIP(const std::string& ip) throw() { _ip = ip; }
    void setPort(unsigned short port) throw() { _port = port; }
    const std::string& getIP() const throw() { return _ip; }
    unsigned short getPort() const throw() { return _port; }
    int connect() throw(IOException);
    int connect(const std::string& ip, unsigned short port) throw(IOException);

  public:
    virtual size_t write(const void* v, size_t count) throw(IOException);
    virtual size_t read(void* v, size_t count) throw(IOException);

  private:
    std::string _ip;
    unsigned short _port;

  };

}

#endif

