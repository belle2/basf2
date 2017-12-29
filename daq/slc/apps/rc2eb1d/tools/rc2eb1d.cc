#include "daq/slc/apps/rc2eb1d/Rc2eb1Callback.h"
#include "daq/slc/apps/rc2eb1d/Rc2eb1BridgeCallback.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/Daemon.h>

#include <daq/slc/base/ConfigFile.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (Daemon::start(argv[1], argc, argv, 1, "<config>")) {
    ConfigFile config("slowcontrol", argv[1]);
    std::string host = config.get("nsm.host");
    int port = config.getInt("nsm.port");
    std::string host2 = config.get("nsm.global.host");
    int port2 = config.getInt("nsm.global.port");
    std::string name = config.get("nsm.nodename");
    int timeout = config.getInt("timeout");
    if (timeout <= 0) timeout = 5;
    Rc2eb1Callback* callback = new Rc2eb1Callback(name, config, timeout);
    Rc2eb1BridgeCallback* callback2 = new Rc2eb1BridgeCallback(callback, name);
    NSMNodeDaemon(callback, host, port, callback2, host2, port2).run();
  }
  return 0;
}
