#include "daq/slc/readout/RunLogMessanger.h"
#include "daq/slc/base/Debugger.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <cstdio>
#include <cerrno>

int main(int argc, char** argv)
{
  const char* path = argv[1];
  Belle2::RunLogMessanger msg;
  msg.open(path);
  int priority = 1;
  if (!msg.send(priority, "READY")) {
    perror("write");
  }
  return 0;
}
/*

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <cstdio>
#include <cerrno>

int send(int fd, const void* buf, size_t count)
{
  size_t c = 0;
  int ret;
  errno = 0;
  while (c < count) {
    ret = ::write(fd, ((unsigned char*)buf + c), (count - c));
    if (ret <= 0) {
      switch (errno) {
      case EINTR: continue;
      case ENETUNREACH:
      case EHOSTUNREACH:
      case ETIMEDOUT:
  usleep(500);
  continue;
      default:
  return -1;
      }
    }
    c += ret;
  }
  return (int)c;
}

int main(int argc, char** argv) {
  const char* path = argv[1];
  int fd;
  if ((fd = ::open(path, O_RDWR)) < 0) {
    fd = 0;
    perror("open");
    return false;
  }
  int priority = 1;
  int ret = 0;
  if ((ret = send(fd, &priority, sizeof(int))) < 0) {
    perror("write");
  }
  printf("priority = %d\n", priority);
  return 0;
}
*/
