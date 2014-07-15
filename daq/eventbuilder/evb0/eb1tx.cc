#include <sys/types.h>
#include <sys/param.h> /* for MAXHOSTNAMELEN */
#include <netdb.h>
#include <assert.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/file.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>

#include "b2eb_format.h"

#include "log.h"
#include "basf2_tcp.h"

char* fdpath;
char* lockfile;

#define MAX_NEIGHBORS 16
#define MAX_EVSIZE (1024 * 1024)

int ndownstream = 2; /* default */
int nupstream = 0;

basf2_tcp downstream[MAX_NEIGHBORS];
basf2_tcp upstream;

const char* eb0 = "127.0.0.1:5101";

void*
eof_watch(void*)
{
  log("eof watcher start\n");
  while (1) {
    for (int i = 0; i < ndownstream; i++) {
      char buf[1];
      int ret;
      ret = recv(fileno(downstream[i].file()), &buf, 1, MSG_PEEK | MSG_DONTWAIT);
      switch (ret) {
        case 0: /* EOF */
          log("EOF found from downstream[%d]\n", i);
          kill(getpid(), SIGTERM);
          exit(0);
          break;
        case 1:
          log("Unpexpected data comes eb1rx to eb1tx\n");
          break;
        case -1: /* alive */
          break;
      }
      const timespec ts = {1, 0};
      nanosleep(&ts, NULL);
    }
  }
}

void
init_fdpass()
{
  const int max_path_len = 1024;
  fdpath = new char[max_path_len];
  lockfile = new char[max_path_len];

  uid_t uid = getuid();

  snprintf(fdpath, max_path_len, "/tmp/fdpass.%d", uid);
  snprintf(lockfile, max_path_len, "/tmp/fdpass.%d.lock", uid);
}

void
slave(int lockfd)
{
  int ret;
  int slavesock = socket(PF_UNIX, SOCK_DGRAM, 0);
  struct sockaddr_un master;
  struct msghdr msg;
  char msgbuf[CMSG_SPACE(sizeof(int))];
  struct cmsghdr* cmsg;
  struct iovec vec;
  pid_t pid = getpid();

  memset(&master, 0, sizeof(master));
  master.sun_family = AF_UNIX;
  strcpy(master.sun_path, fdpath);

  msg.msg_name = (struct sockaddr*)&master;
  msg.msg_namelen = sizeof(master);

  vec.iov_base = &pid;
  vec.iov_len = sizeof(pid);

  msg.msg_iov = &vec;
  msg.msg_iovlen = 1; /* # of block */

  msg.msg_control = msgbuf;
  msg.msg_controllen = sizeof(msgbuf);

  cmsg = CMSG_FIRSTHDR(&msg);
  assert(cmsg != NULL);
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type = SCM_RIGHTS;
  cmsg->cmsg_len = CMSG_LEN(sizeof(int));
  *(int*)CMSG_DATA(cmsg) = 1;  /* stdout */

  msg.msg_controllen = cmsg->cmsg_len;

  msg.msg_flags = 0;

  ret = sendmsg(slavesock, &msg, 0);
  if (ret == -1) {
    log("failed to sendmsg, %s\n", strerror(errno));
    return;
  }
  log("SCM_RIGHTS sent\n");
  close(slavesock);
  switch (errno) {
    case ECONNREFUSED:
      log("socket exists, but no master process\n");
      unlink(fdpath);
      return;
    case ENOENT:
      log("socket doesn't exist, so there is no master\n");
      return;
    default:
      exit(0);
  }
}

int
compare_neighbor(const void* va, const void* vb)
{
  neighbor* a = (neighbor*)va;
  neighbor* b = (neighbor*)vb;

  return strcmp(a->remote(), b->remote());
}

void
master(int lockfd)
{
  int fd;
  int i;
  int ret;
  int mastersock = socket(PF_UNIX, SOCK_DGRAM, 0);
  struct sockaddr_un me;

  struct msghdr msg;
  struct iovec iov;
  char buffer[1024];
  char msgbuf[CMSG_SPACE(sizeof(int))];


  iov.iov_base = buffer;
  iov.iov_len = sizeof(buffer);

  msg.msg_name = 0;
  msg.msg_namelen = 0;
  msg.msg_iov = & iov;
  msg.msg_iovlen = 1;
  msg.msg_control = msgbuf;
  msg.msg_controllen = sizeof(msgbuf);

  log("Ok, I'll be master\n");

  downstream[0].open(1);

  memset(&me, 0, sizeof(me));
  me.sun_family = AF_UNIX;
  strcpy(me.sun_path, fdpath);

  unlink(fdpath);

  ret = bind(mastersock, (struct sockaddr*)&me, sizeof(me));
  if (ret) {
    log("failed to bind %s", strerror(errno));
  }

  close(lockfd);

  for (int i = 1; i < ndownstream; i++) {
    ret = recvmsg(mastersock, &msg, 0);
    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
    log("ret = %d\n", ret);
    log("type = %d\n", cmsg->cmsg_type);
    assert(cmsg->cmsg_type == SCM_RIGHTS);
    log("fd = %d\n", *(int*)CMSG_DATA(cmsg));
    log("from pid %d\n", *(pid_t*)buffer);

    /* this file descriptor is stdout which was passed from slave */

    fd = *(int*)CMSG_DATA(cmsg);

    downstream[i].open(fd);
  }

  close(mastersock);

  qsort(downstream, ndownstream, sizeof(downstream[0]), compare_neighbor);

  for (int i = 0; i < ndownstream; i++) {
    log("client[%d] fd:%d remote:%s\n",
        i,
        fileno(downstream[i].file()),
        downstream[i].remote()
       );
  }
}

int
main(int argc, char** argv)
{
  int lockfd;
  int ret;
  int ch;

  init_fdpass();

  while ((ch = getopt(argc, argv, "e:i:p:")) != -1) {
    char* prefix;
    switch (ch) {
      case 'e':
        eb0 = optarg;
        break;
      case 'i':
        ndownstream = strtol(optarg, 0, 0);
        break;
      case 'p':
        prefix = optarg;
        ret = chdir(prefix);
        if (ret != 0) {
          log("failed to chdir to %s", prefix);
          abort();
        }
        break;
    }
  }

  // openlog("EB", LOG_NDELAY|LOG_PID, LOG_DAEMON);
  program_name_in_syslog = "eb1tx";

  lockfd = open(lockfile, O_CREAT, 0644);
  assert(lockfd != -1);
  ret = flock(lockfd, LOCK_EX);
  assert(ret == 0);
  slave(lockfd); /* If I can't be slave, try master */
  log("try to become master\n");
  master(lockfd);

  log("all downstreams are connected.\n");

  if (0 == strncmp(eb0, "-", 2)) {
    upstream.open(0, "r");
  } else {
    upstream.open(eb0);
  }

  log("upstream is connected.\n");

  pthread_t eof_watcher;
  pthread_create(&eof_watcher, NULL, eof_watch, NULL);

  for (int event_number = 0;; event_number++) {
    upstream.fetch_event();
    neighbor& dest = downstream[event_number % ndownstream];

    if (event_number < 10 ||
        (event_number < 1000 && event_number % 100 == 0) ||
        (event_number < 10000 && event_number % 1000 == 0) ||
        event_number % 10000 == 0) {
      log("event_no = %d\n", event_number);
    }

    upstream.send_event(dest.file());
  }
}

