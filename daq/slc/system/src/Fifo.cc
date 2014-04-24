#include "daq/slc/system/Fifo.h"

#include <daq/slc/base/StringUtil.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <cstdio>

using namespace Belle2;

Fifo Fifo::mkfifo(const std::string& path) throw(IOException)
{
  ::mkfifo(path.c_str(), 0666);
  Fifo fifo;
  fifo.open(path.c_str());
  return fifo;
}

void Fifo::open(const std::string& path, const std::string& mode_s)
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
    throw (IOException("Failed to open fifo."));
  }
}

void Fifo::unlink(const std::string& path) throw(IOException)
{
  if ((::unlink(path.c_str())) < 0) {
    perror("unlink");
  }
  close();
}

size_t Fifo::write(const void* buf, size_t count) throw(IOException)
{
  return ::write(_fd, buf, count);
}

size_t Fifo::read(void* buf, size_t count) throw(IOException)
{
  return ::read(_fd, buf, count);
}
