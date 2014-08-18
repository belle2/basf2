#include "daq/slc/system/UDPSocket.h"

#include <daq/slc/base/StringUtil.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>

#include <sstream>
#include <vector>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>

using namespace Belle2;

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
    m_addr.sin_addr.s_addr |= 0xFF << 24;
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
    m_addr.sin_addr.s_addr |= 0xFF << 24;
    int yes = 1;
    setsockopt(m_fd, SOL_SOCKET, SO_BROADCAST,
               (char*)&yes, sizeof(yes));
  }
}

int UDPSocket::bind(unsigned int port, const std::string& hostname)
throw (IOException)
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
  }
  return bind();
}

int UDPSocket::bind() throw (IOException)
{
  if (::bind(m_fd, (struct sockaddr*)&m_addr, sizeof(m_addr)) < 0) {
    close();
    throw (IOException("Failed to bind socket port=%d",
                       ntohs(m_addr.sin_port)));
  }
  return m_fd;
}

size_t UDPSocket::write(const void* buf, size_t count) throw(IOException)
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

size_t UDPSocket::read(void* buf, size_t count) throw(IOException)
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

const std::string UDPSocket::getHostName() const throw()
{
  return inet_ntoa(m_addr.sin_addr);
}

unsigned int UDPSocket::getAddress() const throw()
{
  return m_addr.sin_addr.s_addr;
}

unsigned int UDPSocket::getPort() const throw()
{
  return ntohs(m_addr.sin_port);
}

const std::string UDPSocket::getRemoteHostName() const throw()
{
  return inet_ntoa(m_remote_addr.sin_addr);
}

unsigned int UDPSocket::getRemoteAddress() const throw()
{
  return m_remote_addr.sin_addr.s_addr;
}

unsigned int UDPSocket::getRemotePort() const throw()
{
  return ntohs(m_remote_addr.sin_port);
}
