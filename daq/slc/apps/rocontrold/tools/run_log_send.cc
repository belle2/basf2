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
  buf.open(path);
  int priority = 1;
  if (!buf.send(priority, "READY")) {
    perror("write");
  }
  return 0;
}
