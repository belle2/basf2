/* merger_merge.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "daq/roisend/util.h"
#include "daq/roisend/b2_socket.h"
#include "daq/roisend/config.h"
#include "daq/roisend/h2m.h"

using namespace std;

#define HRP_MALLOC_BUFFER_SIZE (ROI_MAX_PACKET_SIZE * 64) /* byte */


/* MM_DEBUG<=0 ... no debug, MM_DEBUG==1 ... data dump to file, MM_DEBUG==2 ... data dump to stderr */
#define MM_DEBUG (0)


FILE* MM_debug_fp;


static const int
MM_init_connect_to_onsen(const char* host, const unsigned int port)
{
  int sd, ret;
  struct pollfd fds;


  sd = b2_create_connect_socket(host, port);
  if (sd == -1) {
    ERROR(b2_create_connect_socket);
    return -1;
  }

  fds.fd      = sd;
  fds.events  = POLLOUT;
  fds.revents = 0;
  ret = poll(&fds, 1, NETWORK_ESTABLISH_TIMEOUT * 1000);
  switch (ret) {
    case -1:
      ERROR(poll);
      return -1;

    case  0:
      fprintf(stderr, "%s:%d: connect(): Connection timed out\n", __FILE__, __LINE__);
      return -1;

    case  1: {
      int ret, connection_error;
      socklen_t optlen;

      optlen = sizeof(connection_error);
      ret = getsockopt(sd, SOL_SOCKET, SO_ERROR, &connection_error, &optlen);
      if (ret == -1) {
        ERROR(getsockopt);
        return -1;
      }
      if (connection_error) {
        fprintf(stderr, "%s:%d: connect(): %s\n", __FILE__, __LINE__, strerror(errno));
        return -1;
      }

      break;
    }

    default:
      fprintf(stderr, "%s:%d: poll(): Unexpected error\n", __FILE__, __LINE__);
      return -1;
  }

  ret = b2_timed_blocking_io(sd, NETWORK_IO_TIMEOUT /* secs */);
  if (sd == -1) {
    ERROR(b2_timed_blocking_io);
    return -1;
  }


  return sd;
}


static const int
MM_init_accept_from_hltout2merger(const unsigned int port)
{
  int sd, nd;
  int one = 1, ret;
  struct pollfd fds;


  sd = b2_create_accept_socket(port);
  if (sd == -1) {
    ERROR(b2_create_accept_socket);
    return -1;
  }

  ret = b2_timed_blocking_io(sd, 0);
  if (ret == -1) {
    ERROR(b2_timed_blocking_io);
    return -1;
  }

  ret = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char*)&one, sizeof(int));
  if (ret == -1) {
    ERROR(setsockopt);
    return -1;
  }

  fds.fd      = sd;
  fds.events  = POLLIN;
  fds.revents = 0;
  ret = poll(&fds, 1, NETWORK_ESTABLISH_TIMEOUT * 1000);
  switch (ret) {
    case -1:
      ERROR(poll);
      return -1;

    case  0:
      fprintf(stderr, "%s:%d: accept(): Connection timed out\n", __FILE__, __LINE__);
      return -1;

    case  1: {
      int ret, connection_error;
      socklen_t optlen;

      optlen = sizeof(connection_error);
      ret = getsockopt(sd, SOL_SOCKET, SO_ERROR, &connection_error, &optlen);
      if (ret == -1) {
        ERROR(getsockopt);
        return -1;
      }
      if (connection_error) {
        fprintf(stderr, "%s:%d: accept(): %s\n", __FILE__, __LINE__, strerror(errno));
        return -1;
      }

      break;
    }

    default:
      fprintf(stderr, "%s:%d: poll(): Unexpected error\n", __FILE__, __LINE__);
      return -1;
  }

  nd = accept(sd, NULL, NULL);
  if (nd == -1) {
    ERROR(accept);
    return -1;
  }

  close(sd);

  ret = b2_timed_blocking_io(nd, NETWORK_IO_TIMEOUT /* secs */);
  if (ret == -1) {
    ERROR(b2_timed_blocking_io);
    return -1;
  }


  return nd;
}


static int
MM_get_packet(const int sd_acc, unsigned char* buf)
{
  int ret;
  size_t n_words_from_hltout;
  size_t n_bytes_from_hltout;


  ret = recv(sd_acc, &n_words_from_hltout, sizeof(unsigned int), MSG_PEEK);
  if (ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
    fprintf(stderr, "%s:%d: recv(): Packet receive timed out\n", __FILE__, __LINE__);
    return -1;
  }
  if (ret != sizeof(unsigned int)) {
    fprintf(stderr, "%s:%d: recv(): Unexpected return value (%d)\n", __FILE__, __LINE__, ret);
    return -1;
  }

  n_words_from_hltout = ntohl(n_words_from_hltout);
  n_bytes_from_hltout = n_words_from_hltout * sizeof(unsigned int);

  ret = b2_recv(sd_acc, buf, n_bytes_from_hltout);
  if (ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
    fprintf(stderr, "%s:%d: recv(): Packet receive timed out\n", __FILE__, __LINE__);
    return -1;
  }
  if (ret != n_bytes_from_hltout) {
    fprintf(stderr, "%s:%d: b2_recv(): Unexpected return value (%d)\n", __FILE__, __LINE__, ret);
    return -1;
  }


  return ret;
}


static const int
MM_term_connect_to_onsen(const int sd)
{
  return close(sd);
}


static const int
MM_term_accept_from_hltout2merger(const int sd)
{
  close(sd);
}


const size_t
perl_split_uint16t(char d, const char* string, unsigned short ret[])
{
  char* buf, *p, delim[2];
  size_t len;
  size_t count = 0;


  len = strlen(string);
  buf = (char*)malloc(len);
  strcpy(buf, string);

  delim[0] = d;
  delim[1] = 0;

  p = strtok(buf, delim);
  if (!p) return 0;
  ret[count++] = atoi(p);

  while (p = strtok(NULL, delim)) ret[count++] = atoi(p);

  free(buf);


  return count;
}


int
main(int argc, char* argv[])
{
  int i;
  int n_hltout = 0;
  int sd_acc[MM_MAX_HLTOUT], sd_con;
  /* local test */
  //  char *onsen_host;
  char onsen_host[1024];
  unsigned short onsen_port;
  /* PC test */
  /* const char *onsen_host = "10.10.10.1";
     const unsigned short onsen_port = 1024; */
  /* real ONSEN */
  /* const char *onsen_host = "10.10.10.80";
     const unsigned short onsen_port = 24; */
  unsigned short accept_port[MM_MAX_HLTOUT];


  /* environmental variable check */
  {
    char* p;
    size_t n_ports;

    p = getenv("ROI_ONSEN_HOST");
    if (!p) {
      fprintf(stderr, "%s:%d: main(): export \"ROI_ONSEN_HOST\"\n", __FILE__, __LINE__);
      exit(1);
    }
    strcpy(onsen_host, p);

    p = getenv("ROI_ONSEN_PORT");
    if (!p) {
      fprintf(stderr, "%s:%d: main(): export \"ROI_ONSEN_PORT\"\n", __FILE__, __LINE__);
      exit(1);
    }
    onsen_port = atoi(p);

    p = getenv("ROI_CLIENT_PORTS");
    if (!p) {
      fprintf(stderr, "%s:%d: main(): export \"ROI_CLIENT_PORTS\"\n", __FILE__, __LINE__);
      exit(1);
    }
    n_hltout = perl_split_uint16t(':', p, accept_port);
  }

  /* initialize */
  {
    int i;

    if (MM_DEBUG > 0) {
      char buf[256];
      sprintf(buf, "./data.merger_merge.%05d.dat", getpid());
      MM_debug_fp = MM_DEBUG > 1 ? stderr : fopen(buf, "w");
    }

    signal(SIGPIPE, SIG_IGN);

    sd_con = MM_init_connect_to_onsen(onsen_host, onsen_port);
    if (sd_con == -1) {
      ERROR(MM_init_connect_to_onsen);
      exit(1);
    }
    printf("%s:%d: MM_init_connect_to_onsen(): Connected to ONSEN\n", __FILE__, __LINE__);

    for (i = 0; i < n_hltout; i++) {
      sd_acc[i] = MM_init_accept_from_hltout2merger(accept_port[i]);
      if (sd_acc[i] == -1) {
        ERROR(MM_init_accept_from_hltout2merger);
        exit(1);
      }
      printf("%s:%d: MM_init_connect_to_onsen()[%d]: Connection from HLTOUT\n", __FILE__, __LINE__, i);
    }
  }


  /* forever (recv -> send) */
  for (i = 0;; i++) {
    int j;

    for (j = 0; j < n_hltout; j++) {
      const size_t n_bytes_header  = sizeof(struct h2m_header_t);
      const size_t n_bytes_footer  = sizeof(struct h2m_footer_t);
      size_t n_bytes_from_hltout;
      size_t n_bytes_to_onsen;
      unsigned char* buf;
      unsigned char* ptr_head_to_onsen;

      /* buffer setup */
      {
        buf = (unsigned char*)valloc(n_bytes_header + ROI_MAX_PACKET_SIZE + n_bytes_footer);
        if (!buf) {
          ERROR(valloc);
          exit(1);
        }
      }

      /* packet setup */
      {
        int ret;

        ret = MM_get_packet(sd_acc[j], buf);
        if (ret == -1) {
          fprintf(stderr, "%s:%d: MM_get_packet()[%d]: %s\n", __FILE__, __LINE__, j, strerror(errno));
          exit(1);
        }

        n_bytes_from_hltout = ret;

        if (MM_DEBUG > 0) {
          static int event = 1;
          fprintf(MM_debug_fp, "---- received event [%d]\n", event++);
          dump_binary(MM_debug_fp, buf, n_bytes_from_hltout);
        }
      }

      /* send packet */
      {
        int ret;
        unsigned char* ptr_head_to_onsen = buf + n_bytes_header;

        n_bytes_to_onsen = n_bytes_from_hltout - n_bytes_header - n_bytes_footer;
        ret = b2_send(sd_con, ptr_head_to_onsen, n_bytes_to_onsen);

        if (ret == -1) {
          ERROR(b2_send);
          exit(1);
        }
        if (ret == 0) {
          fprintf(stderr, "%s:%d: b2_send(): Connection closed\n", __FILE__, __LINE__);
          exit(1);
        }

        if (MM_DEBUG > 0) {
          static int event = 1;
          fprintf(MM_debug_fp, "---- sent event [%d]\n", event++);
          dump_binary(MM_debug_fp, ptr_head_to_onsen, n_bytes_to_onsen);
        }
      }

      free(buf);
    }
  }


  /* termination: never reached */
  {
    int i;

    MM_term_connect_to_onsen(sd_con);
    for (i = 0; i < n_hltout; i++) MM_term_accept_from_hltout2merger(sd_acc[i]);
  }


  return 0;
}

