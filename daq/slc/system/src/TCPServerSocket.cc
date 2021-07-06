/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/slc/system/TCPServerSocket.h"

#include <daq/slc/base/IOException.h>

#include <cstdio>
#include <cstring>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

#include <netinet/in.h>
#include <netdb.h>

using namespace Belle2;

int TCPServerSocket::open(const std::string& ip, unsigned short port,
                          int nqueue)
{
  m_ip = ip;
  m_port = port;
  return open(nqueue);
}

int TCPServerSocket::open(int nqueue)
{
  if (m_fd > 0) {
    throw (IOException("Socket is working already."));
  }
  sockaddr_in addr;
  memset(&addr, 0, sizeof(sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(m_port);

  m_fd = socket(PF_INET, SOCK_STREAM, 0);
  if (m_fd == -1) {
    m_fd = 0;
    throw (IOException("Fail to create a server socket."));
  }
  int enable = 1;
  if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == -1) {
    m_fd = 0;
    throw (IOException("Fail to set resue address for the socket."));
  }
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
  if (host == NULL) {
    throw (IOException("Fail to get host ip: %s", m_ip.c_str()));
  }
  addr.sin_addr.s_addr = (*(unsigned long*)host->h_addr_list[0]);

  if (bind(m_fd, (const sockaddr*) & (addr), sizeof(sockaddr_in)) != 0) {
    throw (IOException("Fail to bind the socket. %s:%d", m_ip.c_str(), m_port));
  }
  if (listen(m_fd, nqueue) != 0) {
    throw (IOException("Fail to listen to the socket."));
  }
  return m_fd;
}

TCPSocket TCPServerSocket::accept()
{
  socklen_t len = sizeof(sockaddr_in);
  sockaddr_in addr;
  memset(&addr, 0, sizeof(sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(m_port);
  int fd;
  errno = 0;
  while (true) {
    if ((fd = ::accept(m_fd, (sockaddr*) & (addr), &len)) == -1) {
      switch (errno) {
        case EINTR: continue;
        case EAGAIN: continue;
        default:
          perror("accept");
          throw (IOException("Fail to accept."));
      }
    }
    break;
  }
  TCPSocket s(fd);
  s.m_ip = inet_ntoa(addr.sin_addr);
  s.m_port = ntohs(addr.sin_port);
  return s;
}
