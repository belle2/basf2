//+
// ereco_master.cc
//-
#include "daq/expreco/ERecoMasterCallback.h"
#include "daq/expreco/ERecoRunControlCallback.h"

#include <daq/slc/runcontrol/RCNodeDaemon.h>

#include <daq/slc/base/ConfigFile.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (Daemon::start(argv[1], argc, argv, 1, "<config>")) {
    ConfigFile config("slowcontrol", argv[1]);
    ERecoMasterCallback* callback = new ERecoMasterCallback();
    RCCallback* callback2 = new ERecoRunControlCallback(callback);
    RCNodeDaemon(config, callback, callback2).run();
  }
  return 0;
}
