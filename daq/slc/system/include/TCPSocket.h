/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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
    virtual ~TCPSocket() {}

  private:
    TCPSocket(int fd) : FileDescriptor(fd), m_ip(""), m_port(0) {}

  public:
    void setIP(const std::string& ip) { m_ip = ip; }
    void setPort(unsigned short port) { m_port = port; }
    const std::string& getIP() const { return m_ip; }
    unsigned short getPort() const { return m_port; }
    int connect();
    int connect(const std::string& ip, unsigned short port);
    void setBufferSize(int size);
    void print();
    const std::string getLocalIP();
    int getLocalAddress();
    int getLocalPort();
    unsigned int getAddress();

  public:
    virtual size_t write(const void* v, size_t count);
    virtual size_t read(void* v, size_t count);
    size_t read_once(void* v, size_t count);

  private:
    std::string m_ip;
    unsigned short m_port;

  };

}

#endif

