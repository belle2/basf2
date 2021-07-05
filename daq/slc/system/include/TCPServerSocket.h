/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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
    virtual ~TCPServerSocket() {}

  public:
    void setIP(const std::string& ip) { m_ip = ip; }
    void setPort(unsigned short port) { m_port = port; }
    const std::string& getIP() const { return m_ip; }
    unsigned short getPort() const { return m_port; }
    int open(int nqueue = 5);
    int open(const std::string& ip, unsigned short port, int nqueue = 5);
    TCPSocket accept();

  private:
    std::string m_ip;
    unsigned short m_port;

  };

}

#endif

