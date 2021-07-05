/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/* merger_merge.cc */

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <poll.h>
#include <csignal>
#include <cerrno>
#include <netinet/in.h>
#include <map>
#include <set>
#include <vector>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <string>

#define ROI_MAX_PACKET_SIZE       (16384) /* bytes */

#define NETWORK_ESTABLISH_TIMEOUT (-1)     /* seconds (0 or negative specifies forever wait) */
#define NETWORK_IO_TIMEOUT        (-1)     /* seconds (0 or negative specifies forever wait) */

#include <boost/endian/arithmetic.hpp>

using namespace std;


#define LOG_FPRINTF (fprintf)
#define ERR_FPRINTF (fprintf)
#define ERROR(func) { fprintf(stderr, "[ERROR] %s:%d: "#func"(): %s\n", __FILE__, __LINE__, strerror(errno));}

std::map<int, std::string> myconn;
std::map<int, unsigned int> mycount;
std::set<int> hltused; // we want a sorted list
std::vector<int> hlts; // initialized on run START

std::set<int> triggers;
unsigned int event_number_max = 0;
unsigned int missing_walk_index = 0;

bool got_sigusr1 = false;
bool got_sigusr2 = false;
bool got_sigint = false;
bool got_sigpipe = false;
bool got_sigterm = false;

void
dump_binary(FILE* fp, const void* ptr, const size_t size)
{
  const unsigned int* p = (const unsigned int*)ptr;
  const size_t _size = size / sizeof(unsigned int);


  for (size_t i = 0; i < _size; i++) {
    fprintf(fp, "%08x ", p[i]);
    if (i % 8 == 7) fprintf(fp, "\n");
  }
  if (_size % 8 != 0) fprintf(fp, "\n");
}


static void catch_usr1_function(int /*signo*/)
{
//     puts("SIGUSR1 caught\n");
  got_sigusr1 = true;
}

static void catch_usr2_function(int /*signo*/)
{
//     puts("SIGUSR2 caught\n");
  got_sigusr2 = true;
}

static void catch_int_function(int /*signo*/)
{
//     puts("SIGINT caught\n");
  got_sigint = true;
}

static void catch_term_function(int /*signo*/)
{
//     puts("SIGTERM caught\n");
  got_sigterm = true;
}

static void catch_pipe_function(int /*signo*/)
{
//     puts("SIGPIPE caught\n");
  got_sigpipe = true;
}

void clear_triggers(void)
{
  triggers.clear();
  event_number_max = 0;
  missing_walk_index = 0;
}

void plot_triggers(void)
{
  int hltcount = hltused.size();
  std::map<int, int> modmissing;
  hlts.clear();
  for (auto h : hltused) hlts.push_back(h);
  if (!triggers.empty()) {
    ERR_FPRINTF(stderr, "[RESULT] merger_merge: trigger low=%u high=%u missing %lu delta %u max %u\n", *triggers.begin(),
                *(--triggers.end()),
                triggers.size(), *(--triggers.end()) - *triggers.begin(), event_number_max);
    int i = 0;
    for (auto& it : triggers) {
      int mod = it % hltcount;
      modmissing[hlts[mod]]++;
      if (i < 100) {
        ERR_FPRINTF(stderr, "[INFO] Miss trig %u (%d) HLT%d\n", it, mod, hlts[mod]);
        i++;
        if (i == 100) {
          ERR_FPRINTF(stderr, "[WARNING] ... too many missing to report\n");
          i++;
        }
      }
    }
  } else {
    ERR_FPRINTF(stderr, "[RESULT] merger_merge: missing triggers 0\n");
  }
  for (auto m : modmissing) {
    ERR_FPRINTF(stderr, "[INFO] merger_merge: missing triggers from HLT%d: %d\n", m.first, m.second);
  }
}

void check_event_nr(unsigned int event_number)
{
  // this code might not detect missing trigger nr 0
  // it is assumed, that run number change has been checked and handled before
  if (event_number_max < event_number) {
    for (uint32_t e = event_number_max + 1; e < event_number; e ++) {
      triggers.insert(e);
    }
    event_number_max = event_number;
  } else {
    // we dont fill event_number in the if above, thus we dont have to remove it
    triggers.erase(event_number);
  }
  if (triggers.size() > missing_walk_index + 1 && event_number_max - *std::next(triggers.begin(), missing_walk_index + 1) > 100000) {
    missing_walk_index++;
  } else if (missing_walk_index > 0 && event_number_max - *std::next(triggers.begin(), missing_walk_index) < 100000) {
    missing_walk_index--;
  }
}

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



static int
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
      ERR_FPRINTF(stderr, "[ERROR] merger_merge: connect() to ONSEN: Connection timed out\n");
      return -1;

    case  1: {
      int connection_error;
      socklen_t optlen;

      optlen = sizeof(connection_error);
      ret = getsockopt(sd, SOL_SOCKET, SO_ERROR, &connection_error, &optlen);
      if (ret == -1) {
        ERROR(getsockopt);
        return -1;
      }
      if (connection_error) {
        ERR_FPRINTF(stderr, "[ERROR] merger_merge: connect() to ONSEN: %s\n", strerror(errno));
        return -1;
      }

      break;
    }

    default:
      ERR_FPRINTF(stderr, "[ERROR] merger_merge: poll() connect to ONSEN: Unexpected error\n");
      return -1;
  }

  ret = b2_timed_blocking_io(sd, NETWORK_IO_TIMEOUT /* secs */);
  if (ret == -1) {
    ERROR(b2_timed_blocking_io);
    return -1;
  }


  return sd;
}


static int
MM_init_accept_from_hltout2merger(const unsigned int port)
{
  int sd;
  int one = 1, ret;
  // struct pollfd fds;


  LOG_FPRINTF(stderr, "[INFO] merger_merge: Waiting for connection from hltout2merger on port %d\n", port);

  sd = b2_create_accept_socket(port);
  if (sd == -1) {
    ERROR(b2_create_accept_socket);
    return -1;
  }

  ret = b2_timed_blocking_io(sd,
                             1); // This means, if the socket blocks longer than Xs, it will return a EAGAIN or EWOULDBLOCK (immediately)
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
  unsigned int header[2] = {}; // length is second word, thus read two

  int ret = recv(sd_acc, &header, sizeof(unsigned int) * 2, MSG_PEEK);
  if (ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
    ERR_FPRINTF(stderr, "[ERROR] merger_merge: recv(): Packet receive timed out\n");
    return -1;
  }
  if (ret != 2 * sizeof(unsigned int)) {
    ERR_FPRINTF(stderr, "[ERROR] merger_merge: recv(): Unexpected return value (%d)\n", ret);
    return -2;
  }

  /// TODO: check the first word to be the correct magic, but for TCP/IP this is overkill
  /// anyway unclear how to recover from a misalignment in data stream

  size_t n_bytes_from_hltout = 2 * sizeof(unsigned int) + ntohl(header[1]);// OFFSET_LENGTH = 1

  ret = b2_recv(sd_acc, buf, n_bytes_from_hltout);
  if (ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
    ERR_FPRINTF(stderr, "[ERROR] merger_merge: b2_recv(): Packet receive timed out\n");
    return -1;
  }
  if (size_t(ret) != n_bytes_from_hltout) {
    ERR_FPRINTF(stderr, "[ERROR] merger_merge: b2_recv(): Unexpected return value (%d)\n", ret);
    return -2;
  }

  return ret;
}


static int
MM_term_connect_to_onsen(const int sd)
{
  return close(sd);
}

void print_stat(void)
{
  ERR_FPRINTF(stderr, "[INFO] --- STAT START ---\n");
  unsigned int sum = 0;
  for (auto& it : mycount) {
    ERR_FPRINTF(stderr, "[INFO] (%s): %d\n", myconn[it.first].c_str(), it.second);
    if (it.first != 0) sum += it.second;
  }
  ERR_FPRINTF(stderr, "[INFO] sum %u out %u diff %d\n", sum, mycount[0], (int)(mycount[0] - sum));
  plot_triggers();
  ERR_FPRINTF(stderr, "[INFO] --- STAT END ---\n");
}

// Main

int
main(int argc, char* argv[])
{
  int current_runnr = -1; // problem: handover without abort
  // int n_hltout = 0;
  int sd_acc = -1;
  int sd_con = -1;
  int need_reconnection_to_onsen = 1;
  int event_count = 0;
  int connected_hlts = 0;
  bool stop_running = false;

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

  LOG_FPRINTF(stderr, "[INFO] merger_merge: Process invoked [ver(%s %s)]\n", __DATE__, __TIME__);

  if (argc < 4) {
    ERR_FPRINTF(stderr, "[ERROR] merger_merge: Usage: merger_merge onsen-host onsen-port client-port#1[:client-port#2[:...]]\n");
    exit(1);
  }

  /* argv copy */
  char* p;

//   p = argv[1];
//   strcpy(shmname, p);
//
//   p = argv[2];
//   shmid = atoi(p);

  p = argv[3];
  strcpy(onsen_host, p);

  p = argv[4];
  onsen_port = atoi(p);

  p = argv[5];
  accept_port = atoi(p);

  signal(SIGPIPE, catch_pipe_function);
  signal(SIGTERM, catch_term_function);
  signal(SIGINT, catch_int_function);
  signal(SIGUSR1, catch_usr1_function);
  signal(SIGUSR2, catch_usr2_function);

  /* Create a port to accept connections*/
  sd_acc = MM_init_accept_from_hltout2merger(accept_port);
  LOG_FPRINTF(stderr, "[INFO] merger_merge: port to accept connections from HLTOUT [%d]\n", sd_acc);


  /* RoI transmission loop */
  // RoI packets
  size_t n_bytes_from_hltout;
  size_t n_bytes_to_onsen;

  unsigned char* buf = (unsigned char*)valloc(ROI_MAX_PACKET_SIZE);
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
          LOG_FPRINTF(stderr, "[INFO] merger_merge: MM_init_connect_to_onsen(): Connected to ONSEN\n");
          connected = true;
          break;
        }

        ERR_FPRINTF(stderr, "[WARNING] merger_merge: connection to onsen failed: reconnect in %d second(s)\n", sleep_sec);
        sleep(sleep_sec);

        /* retry connection */
        continue;
      }
    }
  }

  // Preparation for select()

  // printf("Starting select() loop\n") ;
  fflush(stderr);
  fflush(stdout);

  clear_triggers();

  fd_set allset;
  FD_ZERO(&allset);
  FD_SET(sd_acc, &allset);
  int maxfd = sd_acc;
  int minfd = sd_acc;
  fd_set rset;//, wset;

  // Handle Obtain ROI and send it to ONSEN
  while (!stop_running) {
    memcpy(&rset, &allset, sizeof(rset));
    // memcpy(&wset, &allset, sizeof(wset));

    // struct timeval timeout;
    // timeout.tv_sec = 0; // 1sec
    // timeout.tv_usec = 1000; // 1msec (in microsec)
    //    printf ( "Select(): maxfd = %d, start select...; rset=%x, wset=%x\n", maxfd, rset, wset);
    int rc = select(maxfd + 1, &rset, NULL, NULL, NULL);
    //    printf ( "Select(): returned with %d,  rset = %8.8x\n", rc, rset );
    if (got_sigusr1) {
      got_sigusr1 = false;
      ERR_FPRINTF(stderr, "[INFO] Got SIGUSR1, Run START\n");
      print_stat();
    }
    if (got_sigusr2) {
      got_sigusr2 = false;
      ERR_FPRINTF(stderr, "[INFO] Got SIGUSR2, Run STOP\n");
      print_stat();
    }
    if (got_sigint) {
      got_sigint = false;
      ERR_FPRINTF(stderr, "[INFO] Got SIGINT, ABORT\n");
      print_stat();
    }
    if (got_sigpipe) {
      got_sigpipe = false;
      ERR_FPRINTF(stderr, "[INFO] Got SIGPIPE\n");
    }
    if (got_sigterm) {
      got_sigterm = false;
      ERR_FPRINTF(stderr, "[INFO] Got SIGTERM\n");
    }
    if (rc < 0) {
      perror("select");
      continue;
    } else if (rc == 0) { // timeout
      continue;
    }

    int t;
    if (FD_ISSET(sd_acc, &rset)) {   // new connection
      struct sockaddr_in isa;
      socklen_t i = sizeof(isa);
      getsockname(sd_acc, (struct sockaddr*)&isa, &i);
      if ((t =::accept(sd_acc, (struct sockaddr*)&isa, &i)) < 0) {
        //      m_errno = errno;
        ERR_FPRINTF(stderr, "[ERROR] Error on accepting new connection\n");
        ERR_FPRINTF(stderr, "[ERROR] %s terminated\n", argv[0]);
        return (-1);
      }
      LOG_FPRINTF(stderr, "[INFO] New socket connection t=%d\n", t);
      char address[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &isa.sin_addr, address, sizeof(address));
      connected_hlts++;

      LOG_FPRINTF(stderr, "[INFO] %d is IP <%s>\n", t, address);
      myconn[t] = address;
      // Assume IP4 and take last decimal as HLT number
      char* ptr = strrchr(address, '.');
      if (ptr) {
        int nr = atoi(ptr + 1);
        hltused.insert(nr);
      }

      fflush(stdout);
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
          if (ret < 0) { // -2 will not have a errno set
            if (ret == -1) {
              ERR_FPRINTF(stderr, "[ERROR] merger_merge: MM_get_packet()[%d]: %s\n", fd, strerror(errno));
            } else {
              ERR_FPRINTF(stderr, "[ERROR] merger_merge: MM_get_packet()[%d]\n", fd);
            }
            ERR_FPRINTF(stderr, "[ERROR] Connection from HLT was closed on HLT side (hltout2merge) from %s\n",
                        myconn[fd].c_str());
            /* connection from HLT is lost */
            // using exit here will dump ALL still open connections from other HLTs
            // rethink if we dont exit but process these to limit loss of events.
            // ERR_FPRINTF(stderr, "%s terminated\n", argv[0]);
            // exit(1);
            close(fd);
            FD_CLR(fd, &allset); //opposite of FD_SET
            connected_hlts--;
            if (connected_hlts == 0) stop_running = true;
            ret = 0;
          }
          n_bytes_from_hltout = ret;

          //    printf ( "RoI received : Event count = % d\n", event_count );
          if (ret > 0) {
            mycount[fd]++;
            if (0 /*event_count < 5 || event_count % 100000 == 0*/) {
              LOG_FPRINTF(stderr, "[INFO] merger_merge: ---- [ %d] received event from ROI transmitter\n", event_count);
              LOG_FPRINTF(stderr, "[INFO] merger_merge: MM_get_packet() Returned %ld\n", n_bytes_from_hltout);
              dump_binary(stderr, buf, n_bytes_from_hltout);
            }
          }
        }

        /* send packet */
        if (n_bytes_from_hltout > 0) {
          int ret;
          unsigned char* ptr_head_to_onsen = buf;

          // extract trigger number, run+exp nr and fill it in a table.

          int runnr = 0;
          int eventnr = 0;

          // From ROIpayload.h
          //     enum { OFFSET_MAGIC = 0, OFFSET_LENGTH = 1, OFFSET_HEADER = 2, OFFSET_TRIGNR = 3, OFFSET_RUNNR = 4, OFFSET_ROIS = 5};
          //    enum { HEADER_SIZE_WO_LENGTH = 3, HEADER_SIZE_WITH_LENGTH = 5, HEADER_SIZE_WITH_LENGTH_AND_CRC = 6};

          if (n_bytes_from_hltout >= 6 * 4) {
            auto* iptr = (boost::endian::big_uint32_t*)ptr_head_to_onsen;
            eventnr = iptr[3];
            runnr = (iptr[4] & 0x3FFF00) >> 8;
//             ERR_FPRINTF(stderr, "%08X %08X %08X %08X %08X -> %08X %08X \n",
//                         (unsigned int)iptr[0],(unsigned int) iptr[1],(unsigned int) iptr[2],(unsigned int) iptr[3],(unsigned int) iptr[4], eventnr, runnr);
          } else {
            LOG_FPRINTF(stderr, "[ERROR] merger_merge: packet to small to hold useful header (%ld)\n", n_bytes_from_hltout);
          }

          if (runnr > current_runnr) {
            ERR_FPRINTF(stderr, "[WARNING] merger_merge: run number increases: got %d current %d trig %d\n", runnr, current_runnr,
                        eventnr);
            print_stat();
            clear_triggers();
            current_runnr = runnr;
          } else if (runnr < current_runnr) {
            // got some event from old run
            ERR_FPRINTF(stderr, "[WARNING] merger_merge: got trigger from older run: got %d current %d trig %d\n", runnr, current_runnr,
                        eventnr);
          }

          if (runnr == current_runnr) {
            // seperate if, as we might set it in the if above
            check_event_nr(eventnr);
          } // if we end the if here, we will send out old events to ONSEN!

          n_bytes_to_onsen = n_bytes_from_hltout;
          while (1) {
            ret = b2_send(sd_con, ptr_head_to_onsen, n_bytes_to_onsen);
            if (ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
              ERR_FPRINTF(stderr, "[WARNING] merger_merge: socket buffer full, retry\n");
              sleep(1);// Bad hack, wait a second
            } else break;
          }

          if (ret == -1) {
            ERROR(b2_send);
            need_reconnection_to_onsen = 1;
            event_count = 0;
            ERR_FPRINTF(stderr, "[ERROR] merger_merge: error to send to ONSEN : %s\n", strerror(errno));
            free(buf);
            ERR_FPRINTF(stderr, "[ERROR] %s terminated\n", argv[0]);
            print_stat();
            exit(1);
          }
          if (ret == 0) {
            ERR_FPRINTF(stderr, "[ERROR] merger_merge: b2_send(): Connection closed\n");
            need_reconnection_to_onsen = 1;
            event_count = 0;
            free(buf);
            ERR_FPRINTF(stderr, "[ERROR] Connection to ONSEN was closed on ONSEN side\n");
            ERR_FPRINTF(stderr, "[ERROR] %s terminated\n", argv[0]);
            print_stat();
            exit(1);
          }

          mycount[0]++;
          if (0 /*event_count < 5 || event_count % 10000 == 0*/) {
            LOG_FPRINTF(stderr, "[INFO] merger_merge: ---- [ %d] sent event to ONSEN\n", event_count);
            dump_binary(stderr, ptr_head_to_onsen, n_bytes_to_onsen);
          }
          // } // if we end if here, we will NOT send old events to onsen, but only after we received the first new event
        }
      }
      event_count++;
      if (event_count % 10000 == 0) {
        int hltcount = hltused.size();
        int mod = *triggers.begin() % hltcount;
        if (triggers.empty()) {
          // workaround for empty vector, but keep same structure for monitor parsing (kibana)
          ERR_FPRINTF(stderr, "[INFO] merger_merge: trigger low %u high %u missing %u inflight %lu delta %u max %u low mod %d low HLT %d\n",
                      event_number_max, event_number_max, missing_walk_index, triggers.size(),
                      0, event_number_max, mod, hlts[mod]);
        } else {
          ERR_FPRINTF(stderr, "[INFO] merger_merge: trigger low %u high %u missing %u inflight %lu delta %u max %u low mod %d low HLT %d\n",
                      *triggers.begin(), *(--triggers.end()), missing_walk_index, triggers.size(),
                      *(--triggers.end()) - *triggers.begin(), event_number_max, mod, hlts[mod]);
        }
      }
    }
  }


  /* termination: never reached */
  MM_term_connect_to_onsen(sd_con);

  if (connected_hlts == 0) {
    ERR_FPRINTF(stderr, "[RESULT] Stopped because all HLTs closed connection\n");
  }
  print_stat();
  ERR_FPRINTF(stderr, "[RESULT] %s terminated\n", argv[0]);
  return 0;
}
