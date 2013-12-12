#include "daq/slc/readout/RunLogMessanger.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdio>
#include <sys/select.h>

using namespace Belle2;

const int RunLogMessanger::DEBUG = 0;
const int RunLogMessanger::NOTICE = 1;
const int RunLogMessanger::ERROR = 2;
const int RunLogMessanger::FATAL = 3;

bool RunLogMessanger::open(const std::string& path, const std::string& mode_s)
{
  _path = path;
  int mode = O_RDONLY;
  if (mode_s.find("w") != std::string::npos) {
    mode = O_WRONLY;
    if (mode_s.find("r") != std::string::npos) {
      mode = O_RDWR;
    }
  }
  if ((_fifo = ::open(path.c_str(), mode)) < 0) {
    _fifo = 0;
    perror("open");
    return false;
  }
  return true;
}

bool RunLogMessanger::create(const std::string& path, const std::string& mode_s)
{
  _path = path;
  if (::mkfifo(path.c_str(), 0666) < 0) {
    perror("mkfifo");
    return false;
  }
  return open(path, mode_s);
}

void RunLogMessanger::close()
{
  if (_fifo > 0) ::close(_fifo);
  _fifo = 0;
}

void RunLogMessanger::unlink(const std::string& path)
{
  if ((::unlink(path.c_str())) < 0) {
    perror("unlink");
  }
  close();
}

std::string RunLogMessanger::recieve(int& priority, int timeout)
{
  if (timeout > 0) {
    if (_fifo <= 0) {
      priority = -1;
      return "";
    }
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(_fifo, &fds);
    int ret;
    timeval t = {timeout, 0};
    ret = ::select(FD_SETSIZE, &fds, NULL, NULL, &t);
    if (ret < 0) {
      perror("select");
      priority = -1;
      return "";
    }
    if (!FD_ISSET(_fifo, &fds)) {
      priority = -1;
      return "";
    }
  }
  if (read(&priority, sizeof(int)) < 0) {
    perror("read");
    priority = -1;
    return "";
  }
  int size = 0;
  if (read(&size, sizeof(int)) < 0) {
    perror("read");
    priority = -1;
    return "";
  }
  if (size > 256) {
    printf("too large size of message text:%d\n", size);
    priority = -1;
    return "";
  }
  char buf[256];
  if (read(buf, size) < 0) {
    perror("read");
    priority = -1;
    return "";
  }
  return buf;
}

bool RunLogMessanger::send(int priority, const std::string& message)
{
  if (write(&priority, sizeof(int)) < 0) {
    perror("write");
    return false;
  }
  int size = (int)message.size();
  if (size > 256) {
    printf("too large size of message text:%d\n", size);
    return false;
  }
  if (write(&size, sizeof(int)) < 0) {
    perror("write");
    return false;
  }
  if (write(message.c_str(), size) < 0) {
    perror("write");
    return false;
  }
  return true;
}

int RunLogMessanger::write(const void* buf, size_t count)
{
  size_t c = 0;
  int ret;
  errno = 0;
  while (c < count) {
    ret = ::write(_fifo, ((unsigned char*)buf + c), (count - c));
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

int RunLogMessanger::read(void* buf, size_t count)
{
  size_t c = 0;
  int ret;
  errno = 0;
  while (c < count) {
    ret = ::read(_fifo, ((unsigned char*)buf + c), (count - c));
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
