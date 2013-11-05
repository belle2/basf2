#include "system/Fifo.h"

#include <unistd.h>
#include <cstdio>

using namespace Belle2;

Fifo Fifo::mkfifo(const char* path, int opt) throw(IOException)
{
  ::mkfifo(path, 777);
  Fifo fifo;
  fifo.open(path, opt);
  return fifo;
}

void Fifo::open(const char* path, int opt) throw(IOException)
{
  if ((_fd = ::open(path, opt)) < 0) {
    perror("open");
    throw (IOException(__FILE__, __LINE__, "Failed to open fifo."));
  }
}

size_t Fifo::write(const void* buf, size_t count) throw(IOException)
{
  return ::write(_fd, buf, count);
}

size_t Fifo::read(void* buf, size_t count) throw(IOException)
{
  return ::read(_fd, buf, count);
}
