/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>

#include "daq/dataflow/RSocketLib.h"

#define MAXHOSTNAME 100

using namespace Belle2;


// RSocketSend class

RSocketSend::RSocketSend(u_short p)
{

  struct sockaddr_in sa;
  bzero(&sa, sizeof(struct sockaddr_in));

  m_errno = 0;

  sa.sin_family = AF_INET;
  sa.sin_port = htons(p);

  int s;
  if ((s = ::socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    m_errno = errno;
    perror("SocketRecv::socket");
    return;
  }

  int optval = 1;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, 4);

  int sizeval = D2_SOCKBUF_SIZE;
  setsockopt(s, SOL_SOCKET, SO_SNDBUF, &sizeval, 4);
  setsockopt(s, SOL_SOCKET, SO_RCVBUF, &sizeval, 4);

  if ((bind(s, (struct sockaddr*)&sa, sizeof(sa))) < 0) {
    m_errno = errno;
    perror("RSocketSend::bind");
    return;
  }

  m_sock = s;
  m_port = (int)p;
  m_sender = 0;
  listen(s, 3);
  printf("RSocketSend:: initialized, sock=%d\n", m_sock);

  return;

}

RSocketSend::~RSocketSend()
{
  if (m_sender > 0) {
    shutdown(m_sender, 2);
    ::close(m_sender);
    printf("RSocketSend:: receiving socket %d closed\n", m_sender);
  }

  shutdown(m_sock, 2);
  ::close(m_sock);
  printf("RSocketSend:: connection socket %d closed\n", m_sock);
}

int RSocketSend::accept()
{
  m_errno = 0;
  struct sockaddr_in isa;
  socklen_t i = sizeof(isa);
  getsockname(m_sock, (struct sockaddr*)&isa, &i);

  int t;
  if ((t =::accept(m_sock, (struct sockaddr*)&isa, &i)) < 0) {
    m_errno = errno;
    return (-1);
  }

  printf("RSocketSend:: connection request accepted, sender=%d\n", t);

  m_sender = t;
  return (t);
}

int RSocketSend::close()
{
  //  ::close ( m_sender );
  //  m_sender = 0;
  printf("RSocketSend: destructed, m_sender = %d\n", m_sender);
  return 0;
}

int RSocketSend::examine()
{
  // printf("RSocketSend::examine(): waiting for client connection ...\n");

  m_errno = 0;
  fd_set ready;
  FD_ZERO(&ready);
  FD_SET(m_sock, &ready);
  struct timeval to;
  to.tv_sec = 0;
  to.tv_usec = 0;
  if (select(FD_SETSIZE, &ready, 0, 0, &to) < 0) {
    m_errno = errno;
    perror("select");
    return (-1);
  }
  if (FD_ISSET(m_sock, &ready)) {
    printf("RSocketSend::connected!!!!\n");
    return (1);
  } else
    return (0);
}

int RSocketSend::get(char* data, int len)
{
  //  printf("SocketSend::get()\n");

  m_errno = 0;
  if (m_sender > 0)
    return m_io.get(m_sender, data, len);
  else
    return -1;
}

int RSocketSend::read(char* data, int len)
{
  m_errno = 0;
  if (m_sender > 0)
    return m_io.read_data(m_sender, data, len);
  else
    return -1;
}

int RSocketSend::put(char* data, int len)
{
  // printf("RSocketSend::put (sd = %d)\n", m_sender);
  m_errno = 0;
  if (m_sender > 0)
    return m_io.put(m_sender, data, len);
  else
    return -1;
}

int RSocketSend::put_wordbuf(int* data, int len)
{
  //  printf("RSocketSend::put (sd = %d)\n", m_sender);
  m_errno = 0;
  if (m_sender > 0)
    return m_io.put_wordbuf(m_sender, data, len);
  else
    return -1;
}

int RSocketSend::write(char* data, int len)
{
  m_errno = 0;
  if (m_sender > 0)
    return m_io.write_data(m_sender, data, len);
  else
    return -1;
}


int RSocketSend::sock() const
{
  return m_sock;
}

void RSocketSend::sock(int sockid)
{
  m_sock = sockid;
}

int RSocketSend::sender() const
{
  return m_sender;
}

int RSocketSend::port() const
{
  return m_port;
}

int RSocketSend::err() const
{
  return m_errno;
}

void RSocketSend::interrupt()
{
  m_io.interrupt();
}


// RSocketRecv class

RSocketRecv::RSocketRecv(const char* node, u_short port)
{
  m_errno = 0;
  m_sock = -1;
  //  struct hostent* hp;
  if ((m_hp = gethostbyname(node)) == NULL) {
    m_errno = errno;
    fprintf(stderr,
            "RSocketRecv::gethostbyname(%s): not found\n", node);
    return;
  }

  //  struct sockaddr_in sa;
  bzero(&m_sa, sizeof(m_sa));
  bcopy(m_hp->h_addr, (char*)&m_sa.sin_addr, m_hp->h_length);
  m_sa.sin_family = m_hp->h_addrtype;
  m_sa.sin_port = htons((u_short)port);

  int s;
  m_sock = -1;
  if ((s = socket(m_hp->h_addrtype, SOCK_STREAM, 0)) < 0) {
    m_errno = errno;
    perror("RSocketRecv:socket");
    return;
  }
  int sizeval = D2_SOCKBUF_SIZE;
  setsockopt(s, SOL_SOCKET, SO_SNDBUF, &sizeval, 4);
  setsockopt(s, SOL_SOCKET, SO_RCVBUF, &sizeval, 4);
  int yes = 1;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, 4);

  // Connect
  int maxretry = 1000;
  for (;;) {
    printf("SocketRecv: connect trying %d times\n", 1000 - maxretry + 1);
    int istat = connect(s, (struct sockaddr*)&m_sa, sizeof(m_sa));
    if (istat >= 0) {
      m_sock = s;
      m_port = port;
      strcpy(m_node, node);
      signal(SIGPIPE, SIG_IGN);
      printf("RSocketRecv: initialized, m_sock = %d\n", s);
      return;
    }
    maxretry--;
    if (maxretry == 0) {
      printf("RSocketRecv: connection failed. exitting\n");
      exit(-1);
    }
    sleep(5);
  }

  /*
  int maxretry = 10;
  tryagain:
  if (connect(s, (struct sockaddr*)&m_sa, sizeof(m_sa)) < 0) {
    m_errno = errno;
    perror("RSocketRecv:connect");
    printf("tried to connect to %s, port %d\n", node, port);
    printf("connection error..... m_sock set to %d\n", m_sock);
    if (m_errno == ETIMEDOUT) {
      printf(".... try again after 5 sec. \n");
      maxretry--;
      sleep(5);
      if (maxretry == 0) return;
      goto tryagain;
    }
    return;
  }

  m_sock = s;
  m_port = port;
  strcpy(m_node, node);
  signal(SIGPIPE, SIG_IGN);
  printf("RSocketRecv: initialized, m_sock = %d\n", s);
  */
}

RSocketRecv::~RSocketRecv()
{
  shutdown(m_sock, 2);
  ::close(m_sock);
  printf("RSocketRecv: destructed, m_sock = %d\n", m_sock);
}

int RSocketRecv::reconnect(int ntry)
{
  // Close existing socket once.
  shutdown(m_sock, 2);
  ::close(m_sock);

  // Setup socket parameters again;
  bzero(&m_sa, sizeof(m_sa));
  bcopy(m_hp->h_addr, (char*)&m_sa.sin_addr, m_hp->h_length);
  m_sa.sin_family = m_hp->h_addrtype;
  m_sa.sin_port = htons((u_short)m_port);

  // Reopen the socket
  int s;
  m_sock = -1;
  if ((s = socket(m_hp->h_addrtype, SOCK_STREAM, 0)) < 0) {
    m_errno = errno;
    perror("RSocketRecv:socket");
    return -3;
  }
  int sizeval = D2_SOCKBUF_SIZE;
  setsockopt(s, SOL_SOCKET, SO_SNDBUF, &sizeval, 4);
  setsockopt(s, SOL_SOCKET, SO_RCVBUF, &sizeval, 4);
  int yes = 1;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, 4);

  m_sock = s;

  // Connect again
  int maxretry = ntry;
  //  printf ("RSocketRecv: reconnecting socket %d, try %d times with 5sec. interval.\n", m_sock, ntry );

  for (;;) {
    printf("RSocketRecv: reconnecting (trial %d) \n", ntry - maxretry + 1);
    int istat = connect(m_sock, (struct sockaddr*)&m_sa, sizeof(m_sa));
    if (istat >= 0) {
      printf("RSocketRecv: reconnected\n");
      return 0;
    }
    maxretry--;
    if (maxretry == 0) return -1;
    sleep(5);
  }
  printf("RSocketRecv: m_sock = %d reconnected.\n", m_sock);
}

int RSocketRecv::get(char* data, int len)
{
  m_errno = 0;
  return m_io.get(m_sock, data, len);
}

int RSocketRecv::get_wordbuf(int* data, int len)
{
  //  printf("RSocketRecv::get_wordbuf()\n");

  m_errno = 0;
  if (m_sock > 0)
    return m_io.get_wordbuf(m_sock, data, len);
  else
    return -1;
}

int RSocketRecv::read(char* data, int len)
{
  m_errno = 0;
  return m_io.read_data(m_sock, data, len);
}

int RSocketRecv::put(char* data, int len)
{
  m_errno = 0;
  // printf("RSocketRecv::put (sd = %d)\n", m_sock);
  return m_io.put(m_sock, data, len);
}

int RSocketRecv::write(char* data, int len)
{
  m_errno = 0;
  return m_io.write_data(m_sock, data, len);
}

char* RSocketRecv::node()
{
  return m_node;
}

int RSocketRecv::port()
{
  return m_port;
}

int RSocketRecv::sock()
{
  return m_sock;
}

void RSocketRecv::sock(int sockid)
{
  m_sock = sockid;
}

int RSocketRecv::err()
{
  return m_errno;
}








