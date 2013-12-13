#include "daq/slc/system/File.h"

#include "daq/slc/base/StringUtil.h"
#include "daq/slc/base/Debugger.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <cstdio>

using namespace Belle2;

void File::open(const std::string& path, const std::string& mode_s)
throw(IOException)
{
  int mode = O_RDONLY;
  if (mode_s.find("w") != std::string::npos) {
    mode = O_WRONLY;
    if (mode_s.find("r") != std::string::npos) {
      mode = O_RDWR;
    }
  }
  if ((_fd = ::open(path.c_str(), mode)) < 0) {
    perror("open");
    Belle2::debug("Failed to open fifo : '%s'", path.c_str());
    throw (IOException(__FILE__, __LINE__, "Failed to open fifo."));
  }
}

void File::unlink(const std::string& path) throw(IOException)
{
  if ((::unlink(path.c_str())) < 0) {
    perror("unlink");
  }
  close();
}

size_t File::write(const void* buf, size_t count) throw(IOException)
{
  size_t c = 0;
  int ret;
  errno = 0;
  while (c < count) {
    ret = ::write(_fd, ((unsigned char*)buf + c), (count - c));
    if (ret <= 0) {
      switch (errno) {
        case EINTR: continue;
        case ENETUNREACH:
        case EHOSTUNREACH:
        case ETIMEDOUT:
          usleep(500);
          continue;
        default:
          throw (IOException(__FILE__, __LINE__, "Error while writing"));
      }
    }
    c += ret;
  }
  return c;
}

size_t File::read(void* buf, size_t count) throw(IOException)
{
  size_t c = 0;
  int ret;
  errno = 0;
  while (c < count) {
    ret = ::read(_fd, ((unsigned char*)buf + c), (count - c));
    if (ret <= 0) {
      switch (errno) {
        case EINTR: continue;
        case EAGAIN: continue;
        default:
          perror("read");
          throw (IOException(__FILE__, __LINE__, "Error while reading."));
      }
    }
    c += ret;
  }
  return c;
}

