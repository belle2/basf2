#ifndef _B2ARICH_TCPSocket_hh_
#define _B2ARICH_TCPSocket_hh_

#include <string>

#include "daq/slc/system/FileDescriptor.h"

namespace Belle2 {

  class TCPSocket : public FileDescriptor {

    friend class TCPServerSocket;

  public:
    TCPSocket() : m_ip(""), m_port(0) {}
    TCPSocket(const std::string& ip, unsigned short port)
      : m_ip(ip), m_port(port) {}
    virtual ~TCPSocket() throw() {}

  private:
    TCPSocket(int fd) : FileDescriptor(fd), m_ip(""), m_port(0) {}

  public:
    void setIP(const std::string& ip) throw() { m_ip = ip; }
    void setPort(unsigned short port) throw() { m_port = port; }
    const std::string& getIP() const throw() { return m_ip; }
    unsigned short getPort() const throw() { return m_port; }
    int connect() throw(IOException);
    int connect(const std::string& ip, unsigned short port) throw(IOException);
    void setBufferSize(int size) throw(IOException);

  public:
    virtual size_t write(const void* v, size_t count) throw(IOException);
    virtual size_t read(void* v, size_t count) throw(IOException);

  private:
    std::string m_ip;
    unsigned short m_port;

  };

}

#endif

