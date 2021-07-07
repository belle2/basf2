/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/system/TCPSocket.h"

#include <daq/slc/base/IOException.h>

#include <cstdio>
#include <cstring>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>

using namespace Belle2;

int TCPSocket::connect(const std::string& ip, unsigned short port)
{
  m_ip = ip;
  m_port = port;
  return connect();
}

int TCPSocket::connect()
{
  if (m_fd > 0) {
    throw (IOException("Socket is working already."));
  }
  sockaddr_in addr;
  memset(&addr, 0, sizeof(sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(m_port);

  if ((m_fd = ::socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    throw (IOException("Failed to create socket"));
  }
  struct hostent* host = NULL;
  host = gethostbyname(m_ip.c_str());
  if (host == NULL) {
    unsigned long ip_address = inet_addr(m_ip.c_str());
    if ((signed long) ip_address < 0) {
      throw (IOException("Wrong host name or ip"));
    } else {
      host = gethostbyaddr((char*)&ip_address, sizeof(ip_address), AF_INET);
    }
  }
  if (host == NULL) {
    close();
    throw (IOException("Failed to connect host %s:%d",
                       m_ip.c_str(), m_port));
  }
  addr.sin_addr.s_addr = (*(unsigned long*) host->h_addr_list[0]);

  if (::connect(m_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    close();
    throw (IOException("Failed to connect host %s:%d",
                       m_ip.c_str(), m_port));
  }

  return m_fd;
}

void TCPSocket::setBufferSize(int size)
{
  if (size > 0) {
    if (setsockopt(m_fd, SOL_SOCKET, SO_SNDBUF, &size, sizeof(size)) != 0) {
      throw (IOException("failed to SO_SNDBUF: %s\n", strerror(errno)));
    }
    if (setsockopt(m_fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)) != 0) {
      throw (IOException("error on SO_RCVBUF: %s\n", strerror(errno)));
    }
  }
}

size_t TCPSocket::write(const void* buf, size_t count)
{
  size_t c = 0;
  int ret;
  while (c < count) {
    errno = 0;
    ret = send(m_fd, ((unsigned char*)buf + c), (count - c), MSG_NOSIGNAL);
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

size_t TCPSocket::read(void* buf, size_t count)
{
  size_t c = 0;
  int ret;
  while (c < count) {
    errno = 0;
    ret = recv(m_fd, ((unsigned char*)buf + c), (count - c), 0);
    if (ret <= 0) {
      switch (errno) {
        case EINTR: continue;
        case EAGAIN: continue;
        default:
          throw (IOException("TCPSocket::read Error while reading."));
      }
    }
    c += ret;
  }
  return c;
}

size_t TCPSocket::read_once(void* buf, size_t count)
{
  int ret;
  errno = 0;
  while (true) {
    ret = recv(m_fd, buf, count, 0);
    if (ret <= 0) {
      switch (errno) {
        case EINTR: continue;
        case EAGAIN: continue;
        default:
          throw (IOException("TCPSocket::read_once Error while reading."));
      }
    }
    break;
  }
  return ret;
}

void TCPSocket::print()
{
  sockaddr_in sa;
  memset(&sa, 0, sizeof(sockaddr_in));
  socklen_t sa_len = sizeof(sa);
  if (getsockname(m_fd, (struct sockaddr*)&sa, (socklen_t*)&sa_len) != 0) {
    perror("getsockname");
  }
  printf("Local IP address is: %s\n", inet_ntoa(sa.sin_addr));
  printf("Local port is: %d\n", (int) ntohs(sa.sin_port));
}

const std::string TCPSocket::getLocalIP()
{
  sockaddr_in sa;
  memset(&sa, 0, sizeof(sockaddr_in));
  socklen_t sa_len = sizeof(sa);
  if (getsockname(m_fd, (struct sockaddr*)&sa, (socklen_t*)&sa_len) != 0) {
    return "";
  }
  return inet_ntoa(sa.sin_addr);
}

int TCPSocket::getLocalAddress()
{
  sockaddr_in sa;
  memset(&sa, 0, sizeof(sockaddr_in));
  socklen_t sa_len = sizeof(sa);
  if (getsockname(m_fd, (struct sockaddr*)&sa, (socklen_t*)&sa_len) != 0) {
    return 0;
  }
  return sa.sin_addr.s_addr;
}

int TCPSocket::getLocalPort()
{
  sockaddr_in sa;
  memset(&sa, 0, sizeof(sockaddr_in));
  socklen_t sa_len = sizeof(sa);
  if (getsockname(m_fd, (struct sockaddr*)&sa, (socklen_t*)&sa_len) != 0) {
    return 0;
  }
  return ntohs(sa.sin_port);
}

unsigned int TCPSocket::getAddress()
{
  struct hostent* host = NULL;
  host = gethostbyname(m_ip.c_str());
  if (host == NULL) {
    unsigned long ip_address = inet_addr(m_ip.c_str());
    if ((signed long) ip_address < 0) {
      throw (std::exception());
      throw (IOException("Wrong host name or ip"));
    } else {
      host = gethostbyaddr((char*)&ip_address, sizeof(ip_address), AF_INET);
    }
  }
  return (*(unsigned long*) host->h_addr_list[0]);
}
