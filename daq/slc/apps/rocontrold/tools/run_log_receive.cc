#include "daq/slc/readout/ProcessLogBuffer.h"
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
  Belle2::ProcessLogBuffer buf;
  buf.unlink(path);
  buf.create(path);
  int count = 0;
  while (true) {
    int priority = 0;
    std::string ret = "";
    if ((ret = buf.recieve(priority)).size() == 0) {
      perror("read");
    }
    printf("priority = %s (%d)\n", ret.c_str(), count++);
  }
  return 0;
}

