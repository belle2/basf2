#include "daq/slc/apps/rocontrold/ROCallback.h"

#include <daq/slc/runcontrol/RCNodeDaemon.h>

#include <daq/slc/base/ConfigFile.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (Daemon::start(argv[1], argc, argv, 1, "<config>")) {
    ConfigFile config("slowcontrol", "ropc", argv[1]);
    NSMNode runcontrol(config.get("runcontrol"));
    RCNodeDaemon(config, new ROCallback(runcontrol)).run();
  }
  return 0;
}
