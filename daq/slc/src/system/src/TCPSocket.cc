#include "TCPSocket.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>

//#include <iostream>
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

int TCPSocket::connect(const std::string& ip, unsigned short port)
throw (Belle2::IOException)
{
  _ip = ip;
  _port = port;
  return connect();
}

int TCPSocket::connect() throw (IOException)
{
  if (_fd > 0) {
    throw (IOException(__FILE__, __LINE__, "Socket is working already."));
  }
  sockaddr_in addr;
  memset(&addr, 0, sizeof(sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(_port);

  if ((_fd = ::socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    throw (IOException(__FILE__, __LINE__, "Failed to create socket"));
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

  if (::connect(_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    close();
    throw (IOException(__FILE__, __LINE__, "Failed to connect host"));
  }

  return _fd;
}

size_t TCPSocket::write(const void* buf, size_t count) throw(IOException)
{
  size_t c = 0;
  int ret;
  errno = 0;
  while (c < count) {
    ret = send(_fd, ((unsigned char*)buf + c), (count - c), MSG_NOSIGNAL);
    if (ret <= 0) {
      switch (errno) {
        case EINTR: continue;
        case ENETUNREACH:
        case EHOSTUNREACH:
        case ETIMEDOUT:
          usleep(500);
          continue;
        default:
          throw (IOException(__FILE__, __LINE__, "Error while writing"));
      }
    }
    c += ret;
  }
  return c;
}

size_t TCPSocket::read(void* buf, size_t count) throw(IOException)
{
  size_t c = 0;
  int ret;
  errno = 0;
  while (c < count) {
    ret = recv(_fd, ((unsigned char*)buf + c), (count - c), 0);
    if (ret <= 0) {
      switch (errno) {
        case EINTR: continue;
        case EAGAIN: continue;
        default:
          throw (IOException(__FILE__, __LINE__, "Error while reading."));
      }
    }
    c += ret;
  }
  return c;
}
