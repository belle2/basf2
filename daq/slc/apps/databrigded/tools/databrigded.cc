#include "daq/slc/apps/databrigded/DatabrigdeCallback.h"

#include <daq/slc/runcontrol/RCNodeDaemon.h>

#include <daq/slc/base/ConfigFile.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (Daemon::start(argv[1], argc, argv, 1, "<config>")) {
    ConfigFile config("slowcontrol", argv[1]);
    RCNodeDaemon(config, new DatabrigdeCallback()).run();
  }
  return 0;
}
