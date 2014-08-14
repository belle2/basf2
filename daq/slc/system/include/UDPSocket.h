#ifndef _B2ARICH_UDPSocket_hh_
#define _B2ARICH_UDPSocket_hh_

#include <string>

#include <netinet/in.h>

#include "daq/slc/system/FileDescriptor.h"

namespace Belle2 {

  class UDPSocket : public FileDescriptor {

    friend class TCPServerSocket;

  public:
    UDPSocket(unsigned int port = 0);
    UDPSocket(unsigned int port,
              const std::string& hostname,
              bool boardcast = false);
    virtual ~UDPSocket() throw() {}

  public:
    int bind() throw(IOException);
    int bind(unsigned int port) throw(IOException);
    const std::string getHostName() const throw();
    unsigned int getPort() const throw();
    unsigned int getAddress() const throw();

  public:
    virtual size_t write(const void* v, size_t count) throw(IOException);
    virtual size_t read(void* v, size_t count) throw(IOException);

  private:
    struct sockaddr_in m_addr;

  };

}

#endif

