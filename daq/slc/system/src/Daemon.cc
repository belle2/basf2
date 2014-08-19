#include "daq/slc/system/Daemon.h"

#include <daq/slc/system/LogFile.h>

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

using namespace Belle2;

bool Daemon::start(const char* logfile,
                   int argc, char** argv)
{
  bool isdaemon = false;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-d") == 0) {
      isdaemon = true;
    } else if (strcmp(argv[i], "-h") == 0) {
      LogFile::debug("Usage : %s [-d]", argv[0]);
      return false;
    }
  }
  LogFile::open(logfile);
  if (isdaemon) {
    daemon(0, 0);
  }
  return true;
}


