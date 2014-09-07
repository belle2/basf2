#ifndef _B2ARICH_TCPServerSocket_hh_
#define _B2ARICH_TCPServerSocket_hh_

#include "daq/slc/system/TCPSocket.h"
#include "daq/slc/system/FileDescriptor.h"

namespace Belle2 {

  class TCPServerSocket : public FileDescriptor {

  public:
    TCPServerSocket()
      : m_ip(""), m_port(0) {}
    TCPServerSocket(const std::string& ip, unsigned short port)
      : m_ip(ip), m_port(port) {}
    virtual ~TCPServerSocket() throw() {}

  public:
    void setIP(const std::string& ip) throw() { m_ip = ip; }
    void setPort(unsigned short port) throw() { m_port = port; }
    const std::string& getIP() const throw() { return m_ip; }
    unsigned short getPort() const throw() { return m_port; }
    int open(int nqueue = 5) throw(IOException);
    int open(const std::string& ip, unsigned short port, int nqueue = 5) throw(IOException);
    TCPSocket accept() throw(IOException);

  private:
    std::string m_ip;
    unsigned short m_port;

  };

}

#endif

