/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/* b2_socket.c */


#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>

#include "daq/roisend/util.h"
#include "daq/roisend/b2_socket.h"


int
b2_timed_blocking_io(const int sd, const int timeout /* secs */)
{
  int ret;


  if (timeout > 0) {
    struct timeval tv;

    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    ret = setsockopt(sd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));
    if (ret == -1) {
      ERROR(setsockopt);
      return -1;
    }

    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    ret = setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
    if (ret == -1) {
      ERROR(setsockopt);
      return -1;
    }
  } else if (timeout == 0) {
    ret = fcntl(sd, F_SETFL, O_NDELAY);
    if (ret == -1) {
      ERROR(fcntl);
      return -1;
    }
  } else if (timeout < 0) {
    ret = fcntl(sd, F_GETFL, O_NDELAY);
    if (ret == -1) {
      ERROR(fcntl);
      return -1;
    }
    ret &= ~O_NDELAY;
    ret = fcntl(sd, F_SETFL, ret);
    if (ret == -1) {
      ERROR(fcntl);
      return -1;
    }
  }


  return 0;
}


int
b2_build_sockaddr_in(const char* hostname, const unsigned short port, struct sockaddr_in* in)
{
  memset(in, 0, sizeof(struct sockaddr_in));

  in->sin_family = AF_INET;
  {
    struct hostent* hoste;
    hoste = gethostbyname(hostname);
    if (!hoste) {
      ERROR(gethostbyname);
      return -1;
    }
    in->sin_addr = *(struct in_addr*)(hoste->h_addr);
  }
  in->sin_port = htons(port);


  return 0;
}


static int
b2_create_tcp_socket(void)
{
  int sd, ret, one = 1;


  sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sd == -1) {
    ERROR(socket);
    return -1;
  }

#if 0
  ret = b2_timed_blocking_io(sd, 0);
  if (ret == -1) {
    ERROR(b2_timed_blocking_io);
    return -1;
  }
#endif

  ret = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*)&one, sizeof(int));
  if (ret == -1) {
    ERROR(setsockopt);
    return -1;
  }


  return sd;
}


int /* returns socket descriptor */
b2_create_accept_socket(const unsigned short port) /* in reality DOES NOT accept */
{
  int sd, ret;
  struct sockaddr_in in;


  sd = b2_create_tcp_socket();
  if (sd < 0) {
    ERROR(b2_create_tcp_socket);
    return -1;
  }

  ret = b2_build_sockaddr_in("0.0.0.0", port, &in);
  if (ret == -1) {
    ERROR(b2_build_sockaddr_in);
    return -1;
  }

  ret = bind(sd, (const struct sockaddr*)&in, sizeof(struct sockaddr_in));
  if (ret == -1) {
    ERROR(bind);
    return -1;
  }

  ret = listen(sd, 1);
  if (ret == -1) {
    ERROR(listen);
    return -1;
  }


  return sd;
}


int /* returns socket descriptor */
b2_create_connect_socket(const char* hostname, const unsigned short port)
{
  int sd, ret;
  struct sockaddr_in in;


  sd = b2_create_tcp_socket();
  if (sd < 0) {
    ERROR(b2_create_tcp_socket);
    return -1;
  }

  ret = b2_build_sockaddr_in(hostname, port, &in);
  if (ret == -1) {
    ERROR(b2_build_sockaddr_in);
    return -1;
  }

  ret = connect(sd, (const struct sockaddr*)&in, sizeof(struct sockaddr_in));
  if (ret == -1 && errno != EINPROGRESS) {
    ERROR(connect);
    return -1;
  }


  return sd;
}


int
b2_send(const int sd, const void* buf, const size_t size)
{
  unsigned char* ptr = (unsigned char*)buf;
  size_t n_bytes_remained = size;


  for (;;) {
    int ret, n_bytes_send;

    ret = send(sd, ptr, n_bytes_remained, 0);
    if (ret == -1 && errno != EINTR) {
      ERROR(send);
      return -1;
    }
    if (ret == -1 && errno == EINTR) {
      fprintf(stderr, "%s:%d: recv(): Packet send timed out\n", __FILE__, __LINE__);
      return -1;
    }
    if (ret == 0) {
      fprintf(stderr, "%s:%d: send(): Connection closed\n", __FILE__, __LINE__);
      return -1;
    }

    n_bytes_send      = ret;
    ptr              += n_bytes_send;

    if (n_bytes_remained < size_t(n_bytes_send))
      /* overrun: internal error */
    {
      fprintf(stderr, "%s:%d: send(): Internal error\n", __FILE__, __LINE__);
      return -1;
    }
    n_bytes_remained -= n_bytes_send;

    if (n_bytes_remained == 0)
      /* fully sendout */
    {
      break;
    }
  }


  return size;
}


int
b2_recv(const int sd,       void* buf, const size_t size)
{
  unsigned char* ptr = (unsigned char*)buf;
  size_t n_bytes_remained = size;


  for (;;) {
    int ret, n_bytes_recv;

    ret = recv(sd, ptr, n_bytes_remained, 0);
    if (ret == -1 && (errno != EINTR && errno != EWOULDBLOCK)) {
      ERROR(recv);
      return -1;
    }
    if (ret == -1 && (errno == EINTR || errno == EWOULDBLOCK)) {
      fprintf(stderr, "%s:%d: recv(): Packet receive timed out\n", __FILE__, __LINE__);
      return -1;
    }
    if (ret == 0) {
      fprintf(stderr, "%s:%d: recv(): Connection closed\n", __FILE__, __LINE__);
      return -1;
    }

    n_bytes_recv      = ret;
    ptr              += n_bytes_recv;
    if (n_bytes_remained < size_t(n_bytes_recv))
      /* overrun: internal error */
    {
      fprintf(stderr, "%s:%d: recv(): Internal error\n", __FILE__, __LINE__);
      return -1;
    }
    n_bytes_remained -= n_bytes_recv;

    if (n_bytes_remained == 0)
      /* fully readout */
    {
      break;
    }
  }


  return size;
}


