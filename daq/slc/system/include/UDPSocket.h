#ifndef _B2ARICH_UDPSocket_hh_
#define _B2ARICH_UDPSocket_hh_

#include <string>

#include <netinet/in.h>

#include "daq/slc/system/FileDescriptor.h"

namespace Belle2 {

  class UDPSocket : public FileDescriptor {

    friend class TCPServerSocket;

  public:
    UDPSocket();
    UDPSocket(unsigned int port);
    UDPSocket(unsigned int port,
              const std::string& hostname,
              bool boardcast = false);
    UDPSocket(unsigned int port,
              unsigned int addr,
              bool boardcast = false);
    virtual ~UDPSocket() throw() {}

  public:
    int bind() throw(IOException);
    int bind(unsigned int port,
             const std::string& hostname = "",
             bool broadcast = true)
    throw(IOException);
    const std::string getHostName() const throw();
    unsigned int getPort() const throw();
    unsigned int getAddress() const throw();
    const std::string getRemoteHostName() const throw();
    unsigned int getRemotePort() const throw();
    unsigned int getRemoteAddress() const throw();

  public:
    virtual size_t write(const void* v, size_t count) throw(IOException);
    virtual size_t read(void* v, size_t count) throw(IOException);

  private:
    struct sockaddr_in m_addr;
    struct sockaddr_in m_remote_addr;

  private:
    static unsigned int findSubnet(unsigned int addr);

  };

}

#endif

