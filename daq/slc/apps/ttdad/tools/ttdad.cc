#include "daq/slc/apps/ttdad/TTDACallback.h"
#include "daq/slc/apps/ttdad/TTDMasterCallback.h"

#include <daq/slc/runcontrol/RCNodeDaemon.h>

#include <daq/slc/base/ConfigFile.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (Daemon::start(argv[1], argc, argv, 1, "<config>")) {
    ConfigFile config("slowcontrol", argv[1]);
    TTDACallback callback;
    TTDMasterCallback callback2(&callback);
    RCNodeDaemon(config, &callback, &callback2).run();
  }
  return 0;
}
