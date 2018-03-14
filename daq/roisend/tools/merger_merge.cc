/* merger_merge.cc */

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

#include "daq/rfarm/manager/RFFlowStat.h"

using namespace std;

#define HRP_MALLOC_BUFFER_SIZE (ROI_MAX_PACKET_SIZE * 64) /* byte */


#define LOG_FPRINTF (fprintf)
#define ERR_FPRINTF (fprintf)


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
      ERR_FPRINTF(stderr, "merger_merge: connect(): Connection timed out\n");
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
        ERR_FPRINTF(stderr, "merger_merge: connect(): %s\n", strerror(errno));
        return -1;
      }

      break;
    }

    default:
      ERR_FPRINTF(stderr, "merger_merge: poll(): Unexpected error\n");
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


  LOG_FPRINTF(stderr, "hltout2merger: Waiting for connection on %d\n", port);

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

  /*
  fds.fd      = sd;
  fds.events  = POLLIN;
  fds.revents = 0;
  ret = poll(&fds, 1, NETWORK_ESTABLISH_TIMEOUT * 1000);
  switch (ret) {
    case -1:
      ERROR(poll);
      return -1;

    case  0:
      ERR_FPRINTF(stderr, "merger_merge: accept(): Connection timed out\n");
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
        ERR_FPRINTF(stderr, "merger_merge: accept(): %s\n", strerror(errno));
        return -1;
      }

      break;
    }

    default:
      ERR_FPRINTF(stderr, "merger_merge: poll(): Unexpected error\n");
      return -1;
  }
  */

  /* Skip accept
  nd = accept(sd, NULL, NULL);
  if (nd == -1) {
    ERROR(accept);
    return -1;
  }

  close(sd);

  ret = b2_timed_blocking_io(nd, NETWORK_IO_TIMEOUT / * secs * /);
  if (ret == -1) {
    ERROR(b2_timed_blocking_io);
    return -1;
  }
  */


  return sd;
  //  return nd;
}


static int
MM_get_packet(const int sd_acc, unsigned char* buf)
{
  int ret;
  size_t n_words_from_hltout;
  size_t n_bytes_from_hltout;


  ret = recv(sd_acc, &n_words_from_hltout, sizeof(unsigned int), MSG_PEEK);
  if (ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
    ERR_FPRINTF(stderr, "merger_merge: recv(): Packet receive timed out\n");
    return -1;
  }
  if (ret != sizeof(unsigned int)) {
    ERR_FPRINTF(stderr, "merger_merge: recv(): Unexpected return value (%d)\n", ret);
    return -1;
  }

  n_words_from_hltout = ntohl(n_words_from_hltout);
  n_bytes_from_hltout = n_words_from_hltout * sizeof(unsigned int);

  ret = b2_recv(sd_acc, buf, n_bytes_from_hltout);
  if (ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
    ERR_FPRINTF(stderr, "merger_merge: recv(): Packet receive timed out\n");
    return -1;
  }
  if (ret != n_bytes_from_hltout) {
    ERR_FPRINTF(stderr, "merger_merge: b2_recv(): Unexpected return value (%d)\n", ret);
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

// Main

int
main(int argc, char* argv[])
{
  int j;
  int n_hltout = 0;
  int sd_acc = -1;
  int sd_con = -1;
  int need_reconnection_to_onsen = 1;
  int event_count = 0;

  char onsen_host[1024];
  unsigned short onsen_port;
  /* PC test */
  /* strcpy(onsen_host, "10.10.10.1");  */
  /* onsen_port = 1024; */
  /* real ONSEN */
  /* strcpy(onsen_host, "10.10.10.80"); */
  /* onsen_port = 24;   */
  //  unsigned short accept_port[MM_MAX_HLTOUT];
  unsigned short accept_port;

  char shmname[1024];
  int shmid;

  LOG_FPRINTF(stderr, "merger_merge: Process invoked [ver(%s %s)]\n", __DATE__, __TIME__);

  if (argc < 4) {
    ERR_FPRINTF(stderr, "merger_merge: Usage: merger_merge onsen-host onsen-port client-port#1[:client-port#2[:...]]\n");
    exit(1);
  }

  /* argv copy */
  char* p;
  size_t n_ports;

  p = argv[1];
  strcpy(shmname, p);

  p = argv[2];
  shmid = atoi(p);

  p = argv[3];
  strcpy(onsen_host, p);

  p = argv[4];
  onsen_port = atoi(p);

  p = argv[5];
  //    n_hltout = perl_split_uint16t(':', p, accept_port);
  accept_port = atoi(p);

  /* Flow monitor */
  Belle2::RFFlowStat* flstat = new Belle2::RFFlowStat(shmname, shmid, NULL);

  /* Create a port to accept connections*/
  signal(SIGPIPE, SIG_IGN);
  sd_acc = MM_init_accept_from_hltout2merger(accept_port);
  LOG_FPRINTF(stderr, "merger_merge: port to accept connections from HLTOUT [%d]\n", sd_acc);


  /* RoI transmission loop */
  // RoI packets
  const size_t n_bytes_header  = sizeof(struct h2m_header_t);
  const size_t n_bytes_footer  = sizeof(struct h2m_footer_t);
  size_t n_bytes_from_hltout;
  size_t n_bytes_to_onsen;
  unsigned char* ptr_head_to_onsen;

  unsigned char* buf = (unsigned char*)valloc(n_bytes_header + ROI_MAX_PACKET_SIZE + n_bytes_footer);
  if (!buf) {
    ERROR(valloc);
    exit(1);
  }

  // Loop forever for ONSEN connection
  bool connected = false;
  while (!connected) {
    // Connect to ONSEN if not
    if (need_reconnection_to_onsen) {
      /* in case of sd_con is connected, disconnect it */
      if (sd_con != -1) close(sd_con);

      /* connect to onsen untill connected */
      for (;;) {
        int sleep_sec = 2;
        sd_con = MM_init_connect_to_onsen(onsen_host, onsen_port);
        // sd_con = 6;
        if (sd_con != -1) {
          /* connected: move to the recv->send loop */
          need_reconnection_to_onsen = 0;
          event_count = 0;
          LOG_FPRINTF(stderr, "merger_merge: MM_init_connect_to_onsen(): Connected to ONSEN\n");
          connected = true;
          break;
        }

        ERR_FPRINTF(stderr, "merger_merge: connection to onsen failed: reconnect in %d second(s)\n", sleep_sec);
        sleep(sleep_sec);

        /* retry connection */
        continue;
      }
    }
  }

  // Preparation for select()

  printf("Starting select() loop\n") ;
  fflush(stdout);

  fd_set allset;
  FD_ZERO(&allset);
  FD_SET(sd_acc, &allset);
  int maxfd = sd_acc;
  int minfd = sd_acc;
  fd_set rset, wset;

  // Handle Obtain ROI and send it to ONSEN
  for (;;) {
    memcpy(&rset, &allset, sizeof(rset));
    memcpy(&wset, &allset, sizeof(wset));

    struct timeval timeout;
    timeout.tv_sec = 0; // 1sec
    timeout.tv_usec = 1000; // 1msec (in microsec)
    //    printf ( "Select(): maxfd = %d, start select...; rset=%x, wset=%x\n", maxfd, rset, wset);
    int rc = select(maxfd + 1, &rset, NULL, NULL, NULL);
    //    printf ( "Select(): returned with %d,  rset = %8.8x\n", rc, rset );
    if (rc < 0) {
      perror("select");
      continue;
    } else if (rc == 0) // timeout
      continue;

    int t;
    if (FD_ISSET(sd_acc, &rset)) {   // new connection
      struct sockaddr_in isa;
      socklen_t i = sizeof(isa);
      getsockname(sd_acc, (struct sockaddr*)&isa, &i);
      if ((t =::accept(sd_acc, (struct sockaddr*)&isa, &i)) < 0) {
        //      m_errno = errno;
        return (-1);
      }
      printf("New socket connection t=%d\n", t);
      FD_SET(t, &allset);
      if (minfd == sd_acc) minfd = t;
      if (t > maxfd) maxfd = t;
      continue;
    } else {
      for (int fd = minfd; fd < maxfd + 1; fd++) {
        n_bytes_from_hltout = 0;
        if (FD_ISSET(fd, &rset)) {
          //    printf ( "fd is available for reading = %d\n", fd );
          /* recv packet */
          int ret;
          ret = MM_get_packet(fd, buf);
          if (ret == -1) {
            ERR_FPRINTF(stderr, "merger_merge: MM_get_packet()[%d]: %s\n", j, strerror(errno));
            /* connection from HLT is lost */
            exit(1);
          }
          n_bytes_from_hltout = ret;

//    printf ( "RoI received : Event count = %d\n", event_count );

          if (event_count < 40 || event_count % 10000 == 0) {
            LOG_FPRINTF(stderr, "merger_merge: ---- [%d] received event from ROI transmitter\n", event_count);
            LOG_FPRINTF(stderr, "merger_merge: MM_get_packet() Returned %d\n", n_bytes_from_hltout);
            dump_binary(stderr, buf, n_bytes_from_hltout);
          }
        }

        /* send packet */
        if (n_bytes_from_hltout > 0) {
          int ret;
          unsigned char* ptr_head_to_onsen = buf + n_bytes_header;

          n_bytes_to_onsen = n_bytes_from_hltout - n_bytes_header - n_bytes_footer;
          ret = b2_send(sd_con, ptr_head_to_onsen, n_bytes_to_onsen);

          if (ret == -1) {
            ERROR(b2_send);
            need_reconnection_to_onsen = 1;
            event_count = 0;
            ERR_FPRINTF(stderr, "merger_merge: error to send to ONSEN : %s\n", strerror(errno));
            free(buf);
            exit(1);
          }
          if (ret == 0) {
            ERR_FPRINTF(stderr, "merger_merge: b2_send(): Connection closed\n");
            need_reconnection_to_onsen = 1;
            event_count = 0;
            free(buf);
            exit(1);
          }

          flstat->log(n_bytes_to_onsen);

          if (event_count < 40 || event_count % 10000 == 0) {
            LOG_FPRINTF(stderr, "merger_merge: ---- [%d] sent event to ONSEN\n", event_count);
            dump_binary(stderr, ptr_head_to_onsen, n_bytes_to_onsen);
          }
        }
      }
      event_count++;
    }
  }


  /* termination: never reached */
  int i;
  MM_term_connect_to_onsen(sd_con);

  return 0;
}

