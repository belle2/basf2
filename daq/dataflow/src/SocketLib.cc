//+
// File : socketlib.cc
// Description : member functions for socketlib
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 18 - Aug - 2000
//-

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>

#include "daq/dataflow/SocketLib.h"

#define MAXHOSTNAME 100

using namespace Belle2;

// SocketIO class
SocketIO::SocketIO()
{
}

SocketIO::~SocketIO()
{
}


int SocketIO::put(int sock, char* data, int len)
{
  int to_size = htonl(len);
  // printf("SocketIO::put(%d): sending size...\n",sock);
  int br = write_data(sock, (char*)&to_size, 4);
  if (br < 0) {
    perror("put: sending size");
    return br;
  }
  // printf("SocketIO::put(%d): sending data...\n",sock);
  int bcount = write_data(sock, data, len);
  if (bcount < 0) perror("put: sending data");
  return (bcount);
}

int SocketIO::put_wordbuf(int sock, int* data, int len)
{
  //  printf("SocketIO::put_wordbuf(%d): sending data...\n",sock);
  int bcount = write_data(sock, (char*)data, len * sizeof(int));
  if (bcount < 0) perror("put: sending data");
  if (bcount <= 0)
    return bcount;
  else
    return ((bcount - 1) / 4 + 1);
}

int SocketIO::write_data(int sock, char* data, int len)
{
  errno = 0;
  char* ptr = data;
  int bcount = 0;
  int br = 0;

  //  printf("write_data( sock=%d. data=%p. len=%d )\n", sock, data, len);

  while (bcount < len) {
    if ((br =::write(sock, ptr, len - bcount)) > 0) {
      bcount += br;
      ptr += br;
    }
    if (br < 0) {
      switch (errno) {
        case EINTR:
          return -1;
        // continue;
        case EPIPE:
          return -1;        // connection closed, sigpipe
        case ENETUNREACH:
        case EHOSTUNREACH:
        case ETIMEDOUT:
          usleep(500);
          continue;
        default: return -1;
      }
    }
  }
  return (bcount);
}

int SocketIO::get(int sock, char* data, int len)
{
  int gcount;
  int br = read_data(sock, (char*)&gcount, 4);
  if (br <= 0) return br;
  gcount = ntohl(gcount);
  if (gcount > len) {
    printf("buffer too small : %d(%d)", gcount, len);
    exit(0);
  }
  int bcount = read_data(sock, data, gcount);
  return (bcount);
}

int SocketIO::get_wordbuf(int sock, int* wrdbuf, int len)
{
  int gcount;
  int br = read_data(sock, (char*)wrdbuf, sizeof(int));
  if (br <= 0) return br;
  //  gcount = ntohl(wrdbuf[0]);
  gcount = wrdbuf[0];
  //  printf ( "gcount = %8.8x (%d)\n", gcount, gcount );
  if (gcount > len) {
    printf("buffer too small : %d(%d)", gcount, len);
    exit(0);
  }
  int bcount = read_data(sock, (char*)&wrdbuf[1], (gcount - 1) * sizeof(int));
  //  printf ( "term = %8.8x\n", wrdbuf[gcount-1] );
  return (wrdbuf[0]);
}

int SocketIO::get_pxd(int sock, char* data, int len)
{
#define MAX_PXD_FRAMES  256
  const int headerlen = 8;
  int* pxdheader = (int*) data;
  int* pxdheadertable = (int*) &data[headerlen];
  int framenr = 0, tablelen = 0, datalen = 0;
  int br = read_data(sock, data, headerlen);
  if (br <= 0) return br;
  if (pxdheader[0] != htonl(0xCAFEBABE)) {
    printf("pxdheader wrong : Magic %X , Frames %X \n", pxdheader[0], ntohl(pxdheader[1]));
    exit(0);
  }
  pxdheader[0] = ntohl(pxdheader[0]); /// as this is fixed .... might not be needed if taken care of in unpacker
  framenr = pxdheader[1] = ntohl(pxdheader[1]);
  if (framenr > MAX_PXD_FRAMES) {
    printf("MAX_PXD_FRAMES too small : %d(%d) \n", framenr, MAX_PXD_FRAMES);
    exit(0);
  }
  tablelen = 4 * framenr;
  br = read_data(sock, (char*)&data[headerlen], tablelen);
  if (br <= 0) return br;
  for (int i = 0; i < framenr; i++) {
    pxdheadertable[i] = ntohl(pxdheadertable[i]);
    datalen += (pxdheadertable[i] + 3) & 0xFFFFFFFC;
  }

  if (datalen + headerlen + tablelen > len) {
    printf("buffer too small : %d %d %d(%d) \n", headerlen, tablelen, datalen, len);
    exit(0);
  }
  int bcount = read_data(sock, data + headerlen + tablelen, datalen);
  if (br <= 0) return br;
  return (headerlen + tablelen + bcount);
}

int SocketIO::read_data(int sock, char* data, int len)
{
  char* buf = data;
  int bcount = 0;
  int br = 0;

  while (bcount < len) {
    if ((br =::read(sock, buf, len - bcount)) > 0) {
      bcount += br;
      buf += br;
    }
    fflush(stdout);
    if (br == 0) return 0;
    if (br < 0) {
      switch (errno) {
        case EINTR:
          if (m_int == 1) {
            printf("read: interrupted!\n");
            m_int = 0;
            return -2;
          } else
            //            continue;
            return -1;
        case EAGAIN:
          continue;
        default:
          perror("SocketIO:read");
          fprintf(stderr, "sock = %d, buf=%x, len = %d\n", sock, buf, len - bcount);
          return -1;
      }
    }
  }
  //  printf ( "SocketIO::read_data ended : bcount = %d!!!\n", bcount );
  return (bcount);
}

void SocketIO::interrupt(void)
{
  m_int = 1;
}

// SocketRecv class

SocketRecv::SocketRecv(u_short p)
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

  signal(SIGPIPE, SIG_IGN);    // ignore SIGPIPE
  //  printf ( "SocketRecv: SIGPIPE will be ignored\n" );
  //  fflush ( stdout );

  if ((bind(s, (struct sockaddr*)&sa, sizeof(sa))) < 0) {
    m_errno = errno;
    perror("SocketRecv::bind");
    return;
  }

  m_sock = s;
  m_sender = 0;
  listen(s, 3);
  printf("SocketRecv:: initialized, sock=%d\n", m_sock);

  return;

}

SocketRecv::~SocketRecv()
{
  if (m_sender > 0) {
    shutdown(m_sender, 2);
    ::close(m_sender);
    printf("SocketRecv:: receiving socket %d closed\n", m_sender);
  }

  shutdown(m_sock, 2);
  ::close(m_sock);
  printf("SocketRecv:: connection socket %d closed\n", m_sock);
}

int SocketRecv::reconnect(int ntry)
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

int SocketRecv::accept()
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

  printf("SocketRecv:: connection request accepted, sender=%d\n", t);

  m_sender = t;
  return (t);
}

int SocketRecv::close()
{
  //  ::close ( m_sender );
  //  m_sender = 0;
  printf("SocketRecv: destructed, m_sender = %d\n", m_sender);
  return 0;
}

int SocketRecv::examine()
{
  // printf("SocketRecv::examine(): waiting for client connection ...\n");

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
    printf("SocketRecv::connected!!!!\n");
    return (1);
  } else
    return (0);
}

int SocketRecv::get(char* data, int len)
{
  //  printf("SocketSend::get()\n");

  m_errno = 0;
  if (m_sender > 0)
    return m_io.get(m_sender, data, len);
  else
    return -1;
}

int SocketRecv::get_wordbuf(int* data, int len)
{
  //  printf("SocketSend::get()\n");

  m_errno = 0;
  if (m_sender > 0)
    return m_io.get_wordbuf(m_sender, data, len);
  else
    return -1;
}

int SocketRecv::read(char* data, int len)
{
  m_errno = 0;
  if (m_sender > 0)
    return m_io.read_data(m_sender, data, len);
  else
    return -1;
}

int SocketRecv::put(char* data, int len)
{
  // printf("SocketRecv::put (sd = %d)\n", m_sender);
  m_errno = 0;
  if (m_sender > 0)
    return m_io.put(m_sender, data, len);
  else
    return -1;
}

int SocketRecv::write(char* data, int len)
{
  m_errno = 0;
  if (m_sender > 0)
    return m_io.write_data(m_sender, data, len);
  else
    return -1;
}


int SocketRecv::sock() const
{
  return m_sock;
}

void SocketRecv::sock(int sockid)
{
  m_sock = sockid;
}

int SocketRecv::sender() const
{
  return m_sender;
}

int SocketRecv::err() const
{
  return m_errno;
}

void SocketRecv::interrupt()
{
  m_io.interrupt();
}


// SocketSend class

SocketSend::SocketSend(const char* node, u_short port)
{
  m_errno = 0;
  m_sock = -1;
  struct hostent* hp;
  if ((hp = gethostbyname(node)) == NULL) {
    m_errno = errno;
    fprintf(stderr,
            "SocketSend::gethostbyname(%s): not found\n", node);
    return;
  }

  struct sockaddr_in sa;
  bzero(&sa, sizeof(sa));
  bcopy(hp->h_addr, (char*)&sa.sin_addr, hp->h_length);
  sa.sin_family = hp->h_addrtype;
  sa.sin_port = htons((u_short)port);

  int s;
  m_sock = -1;
  if ((s = socket(hp->h_addrtype, SOCK_STREAM, 0)) < 0) {
    m_errno = errno;
    perror("SocketSend:socket");
    return;
  }
  int sizeval = D2_SOCKBUF_SIZE;
  setsockopt(s, SOL_SOCKET, SO_SNDBUF, &sizeval, 4);
  setsockopt(s, SOL_SOCKET, SO_RCVBUF, &sizeval, 4);

  signal(SIGPIPE, SIG_IGN);    // ignore SIGPIPE
  //  printf ( "SocketSend: SIGPIPE will be ignored\n" );
  //  fflush ( stdout );

  int maxretry = 10;
tryagain:
  if (connect(s, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
    m_errno = errno;
    perror("SocketSend:connect");
    printf("connection error..... m_sock set to %d\n", m_sock);
    if (m_errno == ETIMEDOUT) {
      printf(".... try again after 5 sec. \n");
      maxretry--;
      sleep(5);
      if (maxretry == 0) exit(1);
      goto tryagain;
    }
    return;
  }

  m_sock = s;
  m_port = port;
  strcpy(m_node, node);
  signal(SIGPIPE, SIG_IGN);
  printf("SocketSend: initialized, m_sock = %d\n", s);
}

SocketSend::~SocketSend()
{
  shutdown(m_sock, 2);
  ::close(m_sock);
  printf("SocketSend: destructed, m_sock = %d\n", m_sock);
}

int SocketSend::reconnect(int ntry)
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
    printf("SocketSend: reconnecting (trial %d) \n", ntry - maxretry + 1);
    int istat = connect(m_sock, (struct sockaddr*)&m_sa, sizeof(m_sa));
    if (istat >= 0) {
      printf("SocketSend: reconnected\n");
      return 0;
    }
    maxretry--;
    if (maxretry == 0) return -1;
    sleep(5);
  }
  printf("SocketSend: m_sock = %d reconnected.\n", m_sock);
}

int SocketSend::get(char* data, int len)
{
  m_errno = 0;
  return m_io.get(m_sock, data, len);
}

int SocketSend::get_pxd(char* data, int len)
{
  m_errno = 0;
  return m_io.get_pxd(m_sock, data, len);
}

int SocketSend::read(char* data, int len)
{
  m_errno = 0;
  return m_io.read_data(m_sock, data, len);
}

int SocketSend::put(char* data, int len)
{
  m_errno = 0;
  // printf("SocketSend::put (sd = %d)\n", m_sock);
  return m_io.put(m_sock, data, len);
}

int SocketSend::put_wordbuf(int* data, int len)
{
  m_errno = 0;
  // printf("SocketSend::put (sd = %d)\n", m_sock);
  return m_io.put_wordbuf(m_sock, data, len);
}

int SocketSend::write(char* data, int len)
{
  m_errno = 0;
  return m_io.write_data(m_sock, data, len);
}

char* SocketSend::node()
{
  return m_node;
}

int SocketSend::port()
{
  return m_port;
}

int SocketSend::sock()
{
  return m_sock;
}

void SocketSend::sock(int sockid)
{
  m_sock = sockid;
}

int SocketSend::err()
{
  return m_errno;
}








