#include "daq/slc/system/Daemon.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <sys/types.h>
#include <unistd.h>
#include <cstring>

using namespace Belle2;

bool Daemon::start(const char* title,
                   int argc, char** argv,
                   int nargc, const char* msg)
{
  bool isdaemon = false;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-d") == 0) {
      isdaemon = true;
      nargc++;
    } else if (strcmp(argv[i], "-h") == 0) {
      LogFile::debug("Usage : %s %s [-d]", argv[0], msg);
      return false;
    }
  }
  if (argc < nargc + 1) {
    LogFile::debug("Usage : %s %s [-d]", argv[0], msg);
    return false;
  }
  LogFile::open(StringUtil::form("%s.%s", argv[0], title));
  if (isdaemon) {
    daemon(0, 0);
  }
  return true;
}


