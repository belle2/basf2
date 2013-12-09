#include "daq/slc/system/TCPServerSocket.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>

#include <sstream>
#include <vector>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>

#include <unistd.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>

using namespace Belle2;

int TCPServerSocket::open(std::string& ip, unsigned short port)
throw(IOException)
{
  _ip = ip;
  _port = port;
  return open();
}

int TCPServerSocket::open() throw(IOException)
{
  if (_fd > 0) {
    throw (IOException(__FILE__, __LINE__, "Socket is working already."));
  }
  sockaddr_in addr;
  memset(&addr, 0, sizeof(sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(_port);

  _fd = socket(PF_INET, SOCK_STREAM, 0);
  if (_fd == -1) {
    _fd = 0;
    throw (IOException(__FILE__, __LINE__, "Fail to create a server socket."));
  }
  int enable = 1;
  if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == -1) {
    _fd = 0;
    throw (IOException(__FILE__, __LINE__, "Fail to set resue address for the socket."));
  }
  struct hostent* host = NULL;
  host = gethostbyname(_ip.c_str());
  if (host == NULL) {
    unsigned long ip_address = inet_addr(_ip.c_str());
    if ((signed long) ip_address < 0) {
      throw (std::exception());
      throw (IOException(__FILE__, __LINE__, "Wrong host name or ip"));
    } else {
      host = gethostbyaddr((char*)&ip_address, sizeof(ip_address), AF_INET);
    }
  }
  addr.sin_addr.s_addr = (*(unsigned long*) host->h_addr_list[0]);

  if (bind(_fd, (const sockaddr*) & (addr), sizeof(sockaddr_in)) != 0) {
    throw (IOException(__FILE__, __LINE__, "Fail to bind the socket."));
  }
  if (listen(_fd, 5) != 0) {
    throw (IOException(__FILE__, __LINE__, "Fail to listen to the socket."));
  }
  return _fd;
}

TCPSocket TCPServerSocket::accept() throw(IOException)
{
  socklen_t len = sizeof(sockaddr_in);
  sockaddr_in addr;
  memset(&addr, 0, sizeof(sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(_port);
  int fd;
  errno = 0;
  while (true) {
    if ((fd = ::accept(_fd, (sockaddr*) & (addr), &len)) == -1) {
      switch (errno) {
        case EINTR: continue;
        case EAGAIN: continue;
        default:
          perror("accept");
          throw (IOException(__FILE__, __LINE__, "Fail to accept."));
      }
    }
    break;
  }
  TCPSocket s(fd);
  s._ip = inet_ntoa(addr.sin_addr);
  s._port = _port;
  return s;
}
