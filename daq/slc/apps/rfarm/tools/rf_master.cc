#include "daq/slc/apps/rfarm/RFMasterCallback.h"
#include "daq/slc/apps/rfarm/RFRunControlCallback.h"

#include <daq/slc/runcontrol/RCNodeDaemon.h>

#include <daq/slc/base/ConfigFile.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (Daemon::start(argv[1], argc, argv, 1, "<config>")) {
    ConfigFile config("slowcontrol", argv[1]);
    RFMasterCallback* callback = new RFMasterCallback(config);
    RCCallback* callback2 = new RFRunControlCallback(callback);
    RCNodeDaemon(config, callback, callback2).run();
  }
  return 0;
}
