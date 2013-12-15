#include "daq/slc/readout/ProcessLogBuffer.h"
#include "daq/slc/base/Debugger.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <cstdio>
#include <cerrno>

using namespace Belle2;

int main(int argc, char** argv)
{
  const char* path = argv[1];
  ProcessLogBuffer buf;
  buf.open(path);
  if (!buf.send(SystemLog::INFO, "READY")) {
    perror("write");
  }
  return 0;
}
