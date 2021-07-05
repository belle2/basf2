/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/system/UDPSocket.h"

#include <daq/slc/base/IOException.h>

#include <cstdio>
#include <cstring>

#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ifaddrs.h>

using namespace Belle2;

unsigned int UDPSocket::findSubnet(unsigned int addr)
{
  struct ifaddrs* ifa_list;
  struct ifaddrs* ifa;
  char addrstr[256], netmaskstr[256];
  if (getifaddrs(&ifa_list) != 0) return 0;
  for (ifa = ifa_list; ifa != NULL; ifa = ifa->ifa_next) {
    memset(addrstr, 0, sizeof(addrstr));
    memset(netmaskstr, 0, sizeof(netmaskstr));
    if (ifa->ifa_addr->sa_family == AF_INET &&
        addr == ((struct sockaddr_in*)ifa->ifa_addr)->sin_addr.s_addr) {
      addr |= ~(((struct sockaddr_in*)ifa->ifa_netmask)->sin_addr.s_addr);
    }
  }
  freeifaddrs(ifa_list);
  return addr;
}

UDPSocket::UDPSocket()
{
  m_addr.sin_family = AF_INET;
  m_addr.sin_addr.s_addr = INADDR_ANY;
}

UDPSocket::UDPSocket(unsigned int port)
{
  m_addr.sin_port = htons(port);
  m_addr.sin_family = AF_INET;
  m_addr.sin_addr.s_addr = INADDR_ANY;
  if ((m_fd = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    throw (IOException("Failed to create socket"));
  }
}

UDPSocket::UDPSocket(unsigned int port,
                     const std::string& hostname,
                     bool boardcast)
{
  m_addr.sin_port = htons(port);
  m_addr.sin_family = AF_INET;
  if (hostname.size() > 0) {
    struct hostent* host = NULL;
    host = gethostbyname(hostname.c_str());
    if (host == NULL) {
      unsigned long addr = inet_addr(hostname.c_str());
      if ((signed long) addr < 0) {
        throw (std::exception());
        throw (IOException("Wrong host name or ip"));
      } else {
        host = gethostbyaddr((char*)&addr, sizeof(addr), AF_INET);
      }
    }
    m_addr.sin_addr.s_addr = (*(unsigned long*) host->h_addr_list[0]);
  }
  if ((m_fd = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    throw (IOException("Failed to create socket"));
  }
  if (boardcast) {
    unsigned int addr = m_addr.sin_addr.s_addr;
    if ((addr = findSubnet(addr)) > 0) {
      m_addr.sin_addr.s_addr = addr;
    }
    int yes = 1;
    setsockopt(m_fd, SOL_SOCKET, SO_BROADCAST,
               (char*)&yes, sizeof(yes));
  }
}

UDPSocket::UDPSocket(unsigned int port,
                     unsigned int addr,
                     bool boardcast)
{
  m_addr.sin_port = htons(port);
  m_addr.sin_family = AF_INET;
  m_addr.sin_addr.s_addr = addr;
  if ((m_fd = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    throw (IOException("Failed to create socket"));
  }
  if (boardcast) {
    addr = m_addr.sin_addr.s_addr;
    if ((addr = findSubnet(addr)) > 0) {
      m_addr.sin_addr.s_addr = addr;
    }
    int yes = 1;
    setsockopt(m_fd, SOL_SOCKET, SO_BROADCAST,
               (char*)&yes, sizeof(yes));
  }
}

int UDPSocket::bind(unsigned int port,
                    const std::string& hostname, bool broadcast)
{
  if (m_fd <= 0 && (m_fd = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    throw (IOException("Failed to create socket"));
  }
  m_addr.sin_port = htons(port);
  if (hostname.size() > 0) {
    struct hostent* host = NULL;
    host = gethostbyname(hostname.c_str());
    if (host == NULL) {
      unsigned long addr = inet_addr(hostname.c_str());
      if ((signed long) addr < 0) {
        throw (std::exception());
        throw (IOException("Wrong host name or ip"));
      } else {
        host = gethostbyaddr((char*)&addr, sizeof(addr), AF_INET);
      }
    }
    m_addr.sin_addr.s_addr = (*(unsigned long*) host->h_addr_list[0]);
    if (broadcast) {
      unsigned int addr = m_addr.sin_addr.s_addr;
      if ((addr = findSubnet(addr)) > 0) {
        m_addr.sin_addr.s_addr = addr;
      }
    }
  }
  return bind();
}

int UDPSocket::bind()
{
  if (::bind(m_fd, (struct sockaddr*)&m_addr, sizeof(m_addr)) < 0) {
    close();
    throw (IOException("Failed to bind socket port=%d",
                       ntohs(m_addr.sin_port)));
  }
  return m_fd;
}

size_t UDPSocket::write(const void* buf, size_t count)
{
  size_t c = 0;
  int ret;
  while (c < count) {
    errno = 0;
    ret = sendto(m_fd, ((unsigned char*)buf + c),
                 (count - c), 0,
                 (struct sockaddr*)&m_addr, sizeof(m_addr));
    if (ret <= 0) {
      switch (errno) {
        case EINTR: continue;
        case ENETUNREACH:
        case EHOSTUNREACH:
        case ETIMEDOUT:
          usleep(500);
          continue;
        default:
          throw (IOException("Error while writing"));
      }
    }
    c += ret;
  }
  return c;
}

size_t UDPSocket::read(void* buf, size_t count)
{
  size_t c = 0;
  int ret;
  socklen_t addrlen = sizeof(m_remote_addr);
  while (true) {
    errno = 0;
    ret = recvfrom(m_fd, ((unsigned char*)buf + c), (count - c), 0,
                   (struct sockaddr*)&m_remote_addr, &addrlen);
    if (ret <= 0) {
      switch (errno) {
        case EINTR: continue;
        case EAGAIN: continue;
        default:
          throw (IOException("Error while reading."));
      }
    }
    c += ret;
    break;
  }
  return c;
}

const std::string UDPSocket::getHostName() const
{
  return inet_ntoa(m_addr.sin_addr);
}

unsigned int UDPSocket::getAddress() const
{
  return m_addr.sin_addr.s_addr;
}

unsigned int UDPSocket::getPort() const
{
  return ntohs(m_addr.sin_port);
}

const std::string UDPSocket::getRemoteHostName() const
{
  return inet_ntoa(m_remote_addr.sin_addr);
}

unsigned int UDPSocket::getRemoteAddress() const
{
  return m_remote_addr.sin_addr.s_addr;
}

unsigned int UDPSocket::getRemotePort() const
{
  return ntohs(m_remote_addr.sin_port);
}
