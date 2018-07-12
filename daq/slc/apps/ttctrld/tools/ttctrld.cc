#include "daq/slc/apps/ttctrld/TTCtrlCallback.h"

#include <daq/slc/runcontrol/RCNodeDaemon.h>

#include <daq/slc/base/ConfigFile.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (Daemon::start(argv[1], argc, argv, 1, "<config>")) {
    ConfigFile config("slowcontrol", argv[1]);
    TTCtrlCallback callback(config.getInt("ftsw"), config.get("ttdnode"),
                            config.get("rcnode"));
    RCNodeDaemon(config, &callback).run();
  }
  return 0;
}
