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

UDPSocket::UDPSocket(unsigned int port)
{
  m_addr.sin_port = htons(port);
}

UDPSocket::UDPSocket(unsigned int port,
                     const std::string& hostname,
                     bool boardcast)
{
  m_addr.sin_port = htons(port);
  m_addr.sin_family = AF_INET;
  //m_addr.sin_addr.s_addr = inet_addr(hostname.c_str());
  m_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
  if ((m_fd = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    throw (IOException("Failed to create socket"));
  }
  // if (boardcast) {
  int yes = 1;
  setsockopt(m_fd, SOL_SOCKET, SO_BROADCAST,
             (char*)&yes, sizeof(yes));
  //}
}

int UDPSocket::bind(unsigned int port)
throw (IOException)
{
  m_addr.sin_port = htons(port);
  return bind();
}

int UDPSocket::bind() throw (IOException)
{
  if (m_fd > 0) {
    throw (IOException("Socket is working already."));
  }
  m_addr.sin_family = AF_INET;
  m_addr.sin_addr.s_addr = INADDR_ANY;
  if ((m_fd = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    throw (IOException("Failed to create socket"));
  }
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
  while (true) {
    errno = 0;
    ret = recv(m_fd, ((unsigned char*)buf + c), (count - c), 0);
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
