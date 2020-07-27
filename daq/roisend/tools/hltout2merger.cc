/* hltout_roi_pass.c */

#include <arpa/inet.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <mqueue.h>

#include <cstdlib>

#include "daq/roisend/util.h"
#include "daq/roisend/b2_socket.h"
#include "daq/roisend/config.h"
#include "daq/roisend/h2m.h"

using namespace std;


/* HRP_DEBUG<=0 ... no debug, HRP_DEBUG==1 ... data dump to file, HRP_DEBUG==2 ... data dump to stderr */
#define HRP_DEBUG (0)

/// TODO add proper logging
#define ERR_FPRINTF fprintf

FILE* HRP_debug_fp;


/* connect */
static int
HRP_init_connect_to_merger_merger(const char* host, const unsigned short port)
{
  int ret, sd;
  struct pollfd fds;

  sd = b2_create_connect_socket(host, port);
  if (sd == -1) {
    ERROR(b2_create_connect_socket);
    return -1;
  }

  fds.fd      = sd;
  fds.events  = POLLOUT;
  fds.revents = 0;
  ret = poll(&fds, 1, NETWORK_ESTABLISH_TIMEOUT);
  switch (ret) {
    case -1:
      ERROR(poll);
      return -1;

    case  0:
      fprintf(stderr, "%s:%d: connect(): Connection timed out (%d secs)\n", __FILE__, __LINE__, NETWORK_ESTABLISH_TIMEOUT);
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
        fprintf(stderr, "%s:%d: connect(): %s\n", __FILE__, __LINE__, strerror(errno));
        return -1;
      }

      break;
    }

    default:
      fprintf(stderr, "%s:%d: poll(): Unexpected error\n", __FILE__, __LINE__);
      return -1;
  }

  ret = b2_timed_blocking_io(sd,
                             1);// This means, if the socket blocks longer than Xs, it will return a EAGAIN or EWOULDBLOCK (immediately)
  if (ret == -1) {
    ERROR(b2_timed_blocking_io);
    return -1;
  }


  return sd;
}


static mqd_t
HRP_init_mqueue(char* name)
{
  mqd_t ret;
  struct mq_attr attr;
  mode_t previous_mode;
  //  const char* name = ROI_MQ_NAME;


  //  if (name[0] != '/' || strchr(name + 1, '/')) { errno = EINVAL; return -1;}

  ret = mq_unlink(name);
  if (ret == (mqd_t) - 1 && errno != ENOENT) {
    ERROR(mq_unlink);
    return -1;
  }

  attr.mq_flags   = 0;
  attr.mq_maxmsg  = ROI_MQ_DEPTH;
  attr.mq_msgsize = ROI_MAX_PACKET_SIZE;

  previous_mode = umask(0);
  ret = mq_open(name, O_CREAT | O_RDONLY, 0666, &attr);
  umask(previous_mode);
  if (ret == (mqd_t) - 1) {
    ERROR(mq_open);
    return (mqd_t) - 1;
  }


  return ret;
}


static ssize_t
HRP_build_header(struct h2m_header_t* h, const unsigned char* /*roi*/, const size_t n_bytes_roi)
{
  size_t n_bytes_total = sizeof(struct h2m_header_t) + n_bytes_roi + sizeof(struct h2m_footer_t);
  size_t n_words_total = n_bytes_total / sizeof(unsigned int);


  h->h_n_words           = htonl(n_words_total);
  h->h_n_words_in_header = htonl(sizeof(struct h2m_header_t) / sizeof(unsigned int));
  h->h_reserved[0]       = 0x02701144;
  h->h_reserved[1]       = 0x02410835;
  h->h_marker            = 0x5f5f5f5f;

  return sizeof(struct h2m_header_t);
}


static size_t
HRP_build_footer(struct h2m_footer_t* f, const unsigned char* /*roi*/, const size_t /*n_bytes_roi*/)
{
  f->f_reserved[0]       = 0x02701144;
  f->f_reserved[1]       = 0x02410835;
  f->f_checksum          = 0;
  f->f_marker            = 0xa0a0a0a0;

  return sizeof(struct h2m_footer_t);
}


static ssize_t
HRP_roi_get(const mqd_t mqd, void* buf, const int timeout /* secs */)
{
  mqd_t ret;
  unsigned char* ptr = (unsigned char*)buf;


  if (timeout <= 0) {
    ret = mq_receive(mqd, (char*)ptr, ROI_MAX_PACKET_SIZE, NULL);
  } else {
    time_t t;
    struct timespec ts;

    t = time(NULL);
    ts.tv_sec  = t;
    ts.tv_nsec = 0;
    ts.tv_sec += timeout;

    ret = mq_timedreceive(mqd, (char*)ptr, ROI_MAX_PACKET_SIZE, 0 /* priority */, &ts);
    if (ret == (mqd_t) - 1 && errno == ETIMEDOUT) {
      fprintf(stderr, "%s:%d: mq_timedreceive(): Message time out (%d secs)\n", __FILE__, __LINE__, timeout);
      return (ssize_t) - 1;
    }
  }

  if (ret == (mqd_t) - 1) {
    ERROR(mq_receive);
    return (ssize_t) - 1;
  }

  return (ssize_t)ret;
}


static int
HRP_term_connect_to_merger_merger(const int sd)
{
  return close(sd);
}


static int
HRP_term_mqueue(const mqd_t mqd)
{
  mq_close(mqd);
  mq_unlink(ROI_MQ_NAME);

  return 0;
}


int
main(int argc, char* argv[])
{
  int sd;
  mqd_t mqd[10];
  char merger_host[256];
  unsigned short merger_port;

  int num_queue = 1;
  char qname[10][256];
  strcpy(qname[0], ROI_MQ_NAME);


  if (argc < 3) {
    printf("[ERROR] hltout2merger : mergerhost mergerport\n");
    exit(-1);
  }

  /* environmental variable check */
  {
    char* p;
    p = argv[1];
    //    p = getenv("ROI_MERGER_HOST");
    if (!p) {
      fprintf(stderr, "%s:%d: main(): export \"ROI_MERGER_HOST\"\n", __FILE__, __LINE__);
      ERR_FPRINTF(stderr, "%s terminated\n", argv[0]);
      exit(1);
    }
    strcpy(merger_host, p);

    p = argv[2];
    //    p = getenv("ROI_MERGER_PORT");
    if (!p) {
      fprintf(stderr, "%s:%d: main(): export \"ROI_MERGER_PORT\"\n", __FILE__, __LINE__);
      ERR_FPRINTF(stderr, "%s terminated\n", argv[0]);
      exit(1);
    }
    merger_port = atoi(p);

    if (argc > 3) {
      num_queue = argc - 3;
      for (int i = 0; i < argc - 3; i++) {
        p = argv[i + 3];
        strcpy(qname[i], p);
      }
    }
  }


  /* initialization */
  {
    if (HRP_DEBUG > 0) {
      char buf[256];
      sprintf(buf, "./data.roi.%05d.dat", getpid());
      HRP_debug_fp = HRP_DEBUG > 1 ? stderr : fopen(buf, "w");
    }

    signal(SIGPIPE, SIG_IGN);

    sd = HRP_init_connect_to_merger_merger(merger_host, merger_port);
    if (sd == -1) {
      ERROR(HRP_init_connect_to_merger_merger);
      ERR_FPRINTF(stderr, "%s terminated\n", argv[0]);
      exit(1);
    }
    printf("%s:%d: main(): Connected to MERGER_MERGE\n", __FILE__, __LINE__);

    for (int i = 0; i < num_queue; i++) {
      mqd[i] = HRP_init_mqueue(qname[i]);
      if (mqd[i] == (mqd_t) - 1) {
        ERROR(HRP_init_mqueue);
        ERR_FPRINTF(stderr, "%s terminated\n", argv[0]);
        exit(1);
      }
    }
    printf("%s:%d: main(): Ready to accept RoI\n", __FILE__, __LINE__);
  }

  /* forever (mq_receive -> send) */
  int curqid = 0;
  while (true) {
    unsigned char* ptr_packet;
    size_t n_bytes_packet;
    unsigned char* buf;

    /* buffer setup */
    {
      buf = (unsigned char*)valloc(sizeof(struct h2m_header_t) + ROI_MAX_PACKET_SIZE + sizeof(struct h2m_footer_t));
      if (!buf) {
        ERROR(valloc);
        ERR_FPRINTF(stderr, "%s terminated\n", argv[0]);
        exit(1);
      }
    }

    /* packet setup */
    {
      unsigned char* ptr_roi;
      ssize_t n_bytes_roi;
      unsigned char* ptr_header;
      unsigned char* ptr_footer;

      ptr_roi        = buf + sizeof(struct h2m_header_t);
      n_bytes_roi    = HRP_roi_get(mqd[curqid], ptr_roi, ROI_IO_TIMEOUT);
      if (n_bytes_roi == -1) {
        ERROR(n_bytes_roi);
        exit(1);
      }
      curqid++;
      if (curqid >= num_queue) curqid = 0;

      ptr_header     = buf;
      ssize_t n_bytes_header = HRP_build_header((struct h2m_header_t*)ptr_header, ptr_roi, n_bytes_roi);

      ptr_footer     = buf + n_bytes_header + n_bytes_roi;
      ssize_t n_bytes_footer = HRP_build_footer((struct h2m_footer_t*)ptr_footer, ptr_roi, n_bytes_roi);

      ptr_packet     = buf;
      n_bytes_packet = n_bytes_header + n_bytes_roi + n_bytes_footer;

      if (HRP_DEBUG > 0) {
        static int event = 1;
        fprintf(HRP_debug_fp, "---- mq_received event [%d]\n", event++);
        dump_binary(HRP_debug_fp, ptr_packet, n_bytes_packet);
      }
    }

    /* send packet */
    {
      int ret;

      while (1) {
        ret = b2_send(sd, ptr_packet, n_bytes_packet);
        if (ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
          ERR_FPRINTF(stderr, "[WARNING] hltout2merger: socket buffer full, retry\n");
          sleep(1);// Bad hack, wait a second
        } else break;
      }
      if (ret == -1) {
        ERROR(b2_send);
        ERR_FPRINTF(stderr, "Send to merger (roisenderd) failed.\n%s terminated\n", argv[0]);
        exit(1);
      }
      if (ret == 0) {
        fprintf(stderr, "%s:%d: b2_send(): Connection closed\n", __FILE__, __LINE__);
        ERR_FPRINTF(stderr, "Connection to merger was closed on merger side (roisenderd)\n%s terminated\n", argv[0]);
        exit(1);
      }

      if (HRP_DEBUG > 0) {
        static int event = 1;
        fprintf(HRP_debug_fp, "---- b2_send event [%d]\n", event++);
        dump_binary(HRP_debug_fp, ptr_packet, n_bytes_packet);
      }
    }

    free(buf);
  }

  /* termination: never reached */
  HRP_term_connect_to_merger_merger(sd);
  //  HRP_term_mqueue(mqd);
  for (int i = 0; i < num_queue; i++) {
    mq_close(mqd[i]);
    mq_unlink(qname[i]);
  }

  ERR_FPRINTF(stderr, "%s terminated\n", argv[0]);

  return 0;
}

