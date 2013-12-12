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
  msg.unlink(path);
  msg.create(path);
  int count = 0;
  while (true) {
    int priority = 0;
    std::string ret = "";
    if ((ret = msg.recieve(priority)).size() == 0) {
      perror("read");
    }
    printf("priority = %s (%d)\n", ret.c_str(), count++);
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

int receive(int fd, void* buf, size_t count)
{
  size_t c = 0;
  int ret;
  errno = 0;
  while (c < count) {
    ret = ::read(fd, ((unsigned char*)buf + c), (count - c));
    if (ret <= 0) {
      switch (errno) {
      case EINTR: continue;
      case EAGAIN: continue;
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
  unlink(path);
  int fd = 0;
  if (mkfifo(path, S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP) < 0) {
    perror("open");
    return false;
  }
  if ((fd = ::open(path, O_RDWR)) < 0) {
    fd = 0;
    perror("open");
    return false;
  }
  int priority = 0;
  int ret = 0;
  if ((ret = receive(fd, &priority, sizeof(int))) < 0) {
    perror("read");
  }
  printf("priority = %d\n", priority);
  return 0;
}
*/
