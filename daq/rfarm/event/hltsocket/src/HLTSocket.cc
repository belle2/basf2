/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <daq/rfarm/event/hltsocket/HLTSocket.h>
#include <framework/logging/Logger.h>

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>

#include <chrono>
#include <thread>

using namespace Belle2;

HLTSocket::~HLTSocket()
{
  close(m_listener);
  close(m_socket);
  m_initialized = false;
}

int HLTSocket::put(char* data, int len)
{
  const int to_size = htonl(len);
  const int br = write_data((char*) &to_size, sizeof(int));
  if (br == 0) {
    B2ERROR("Can not write to broken pipe.");
    return 0;
  } else if (br < 0) {
    B2ERROR("Error in sending the size: " << strerror(errno));
    return br;
  }
  const int bcount = write_data(data, len);
  if (bcount == 0) {
    B2ERROR("Can not write to broken pipe.");
    return 0;
  } else if (bcount < 0) {
    B2ERROR("Error in sending the data: " << strerror(errno));
    return bcount;
  }
  B2ASSERT("Written buffer size != buffer size in data!", bcount == len);
  return bcount;
}

int HLTSocket::put_wordbuf(int* data, int len)
{
  // TODO: it is implicitly assumed that the first entry in the buffer is the buffer size!
  const int gcount = data[0];
  B2ASSERT("The first entry in the data must be the buffer size!", gcount == len);

  int bcount = write_data((char*) data, len * sizeof(int));
  if (bcount == 0) {
    B2ERROR("Can not write to broken pipe.");
    return 0;
  } else if (bcount < 0) {
    B2ERROR("Error in sending the data: " << strerror(errno));
    return bcount;
  }
  bcount = ((bcount - 1) / sizeof(int) + 1);

  B2ASSERT("Written buffer size != buffer size in data!", bcount == len);

  // ATTENTION: the returned size is size / 4
  return bcount;
}

int HLTSocket::write_data(char* data, int len)
{
  errno = 0;
  char* ptr = data;
  int bcount = 0;
  int br = 0;

  while (bcount < len) {
    if ((br = ::write(m_socket, ptr, len - bcount)) > 0) {
      bcount += br;
      ptr += br;
    }
    if (br < 0) {
      switch (errno) {
        case EINTR:
          return -1;
        case EPIPE:
          return 0;        // connection closed, sigpipe
        default:
          return -1;
      }
    }
  }
  return bcount;
}

int HLTSocket::get(char* data, int len)
{
  int gcount;
  const int br = read_data((char*) &gcount, 4);
  if (br < 0) {
    B2ERROR("Error in getting the size: " << strerror(errno));
    return br;
  } else if (br == 0) {
    B2WARNING("No data from socket!");
    return 0;
  }

  gcount = ntohl(gcount);
  if (gcount > len) {
    B2ERROR("buffer too small! " << gcount << " < " << len);
    return -1;
  }
  const int bcount = read_data(data, gcount);
  if (bcount < 0) {
    B2ERROR("Error in getting the data: " << strerror(errno));
    return bcount;
  } else if (bcount == 0) {
    B2WARNING("No data from socket!");
    return 0;
  }
  B2ASSERT("Read buffer size != buffer size in data!", bcount == gcount);
  return bcount;
}

int HLTSocket::get_wordbuf(int* wrdbuf, int len)
{
  int br = read_data((char*) wrdbuf, sizeof(int));
  if (br < 0) {
    B2ERROR("Error in getting the size: " << strerror(errno));
    return br;
  } else if (br == 0) {
    B2WARNING("No data from socket!");
    return 0;
  }

  const int gcount = (wrdbuf[0] - 1) * sizeof(int);
  if (gcount > len) {
    B2ERROR("buffer too small! " << gcount << " < " << len);
    return -1;
  }
  // ATTENTION: the send size is size / 4
  const int bcount = read_data((char*) &wrdbuf[1], gcount);
  if (bcount < 0) {
    B2ERROR("Error in getting the data: " << strerror(errno));
    return bcount;
  } else if (bcount == 0) {
    B2WARNING("No data from socket!");
    return 0;
  }

  B2ASSERT("Read buffer size != buffer size in data: " << bcount << " != " << gcount, bcount == gcount);
  return (wrdbuf[0]);
}

int HLTSocket::read_data(char* data, int len)
{
  char* buf = data;
  int bcount = 0;
  int br = 0;

  while (bcount < len) {
    if ((br = ::read(m_socket, buf, len - bcount)) > 0) {
      bcount += br;
      buf += br;
    }
    if (br == 0) return 0;
    if (br < 0) {
      switch (errno) {
        case EINTR:
          return -1;
        case EAGAIN:
          continue;
        default:
          return -1;
      }
    }
  }
  return bcount;
}

bool HLTSocket::accept(unsigned int port)
{
  // Before going on make sure to close the old socket
  close(m_socket);

  // Initialize the first listener
  if (m_listener <= 0) {
    B2RESULT("Started listening for new clients");
    struct sockaddr_in sa;
    bzero(&sa, sizeof(struct sockaddr_in));

    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);

    if ((m_listener = ::socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      B2ERROR("Socket initialization failed: " << strerror(errno));
      return false;
    }

    int optval = 1;
    setsockopt(m_listener, SOL_SOCKET, SO_REUSEADDR, &optval, 4);

    int sizeval = D2_SOCKBUF_SIZE;
    setsockopt(m_listener, SOL_SOCKET, SO_SNDBUF, &sizeval, 4);
    setsockopt(m_listener, SOL_SOCKET, SO_RCVBUF, &sizeval, 4);

    signal(SIGPIPE, SIG_IGN);

    if ((bind(m_listener, (struct sockaddr*) &sa, sizeof(sa))) < 0) {
      B2ERROR("Socket binding failed: " << strerror(errno));
      return false;
    }

    listen(m_listener, 3);
  }

  struct sockaddr_in isa;
  socklen_t i = sizeof(isa);
  getsockname(m_listener, (struct sockaddr*)&isa, &i);

  B2RESULT("Started accepting new clients");
  if ((m_socket =::accept(m_listener, (struct sockaddr*)&isa, &i)) < 0) {
    B2ERROR("Socket accepting failed: " << strerror(errno));
    return false;
  }

  B2RESULT("Accepted connection with socket: " << m_socket);
  m_initialized = true;
  return true;
}

void HLTSocket::close(int socket)
{
  if (socket > 0) {
    shutdown(socket, 2);
    ::close(socket);
    B2RESULT("Socket closed: " << socket);
  }
}

bool HLTSocket::connect(const std::string& hostName, unsigned int port, const HLTMainLoop& mainLoop)
{
  // Before going on make sure to close the old socket
  close(m_socket);

  struct hostent* hp;
  if ((hp = gethostbyname(hostName.c_str())) == NULL) {
    B2ERROR("Host not found: " << strerror(errno));
    return false;
  }

  struct sockaddr_in sa;
  bzero(&sa, sizeof(sa));
  bcopy(hp->h_addr, (char*) &sa.sin_addr, hp->h_length);
  sa.sin_family = hp->h_addrtype;
  sa.sin_port = htons((u_short) port);

  if ((m_socket = socket(hp->h_addrtype, SOCK_STREAM, 0)) < 0) {
    B2ERROR("Socket initialization failed: " << strerror(errno));
    return false;
  }

  int sizeval = D2_SOCKBUF_SIZE;
  setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, &sizeval, 4);
  setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, &sizeval, 4);
  int yes = 1;
  setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &yes, 4);

  signal(SIGPIPE, SIG_IGN);

  using namespace std::chrono_literals;

  int maxretry = 0;
  while (::connect(m_socket, (struct sockaddr*) &sa, sizeof(sa)) < 0) {
    if (errno == ETIMEDOUT or errno == ECONNREFUSED) {
      B2WARNING("Connection failed, will retry in 1 second... " << maxretry);
      std::this_thread::sleep_for(1s);
    } else {
      B2ERROR("Socket initialization failed: " << strerror(errno));
      return false;
    }

    if (not mainLoop.isRunning()) {
      return false;
    }
    maxretry++;
  }

  B2RESULT("Connected with socket: " << m_socket);
  m_initialized = true;
  return true;
}

bool HLTSocket::initialized() const
{
  return m_initialized;
}
void HLTSocket::deinitialize()
{
  m_initialized = false;
}
