#include "daq/slc/apps/flowmonitor/FlowMonitorCallback.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/Daemon.h>

#include <daq/slc/base/ConfigFile.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (Daemon::start("flowmonitor", argc, argv, 0, "<config>")) {
    ConfigFile config("slowcontrol", argv[1]);
    FlowMonitorCallback callback(config.get("nsm.nodename"),
                                 config.get("runcontrol"));
    callback.setNeventsThreshold(config.getInt("nevents"));
    const std::string host = config.get("nsm.host");
    int port = config.getInt("nsm.port");
    NSMNodeDaemon daemon(&callback, host, port);
    daemon.run();
  }
  return 0;
}
