#include "daq/slc/apps/nsmbridged/NsmbridgeCallback.h"
#include "daq/slc/apps/nsmbridged/NsmbridgeMasterCallback.h"

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
    std::string name = config.get("nsm.nodename");
    int timeout = config.getInt("timeout");
    if (timeout <= 0) timeout = 5;
    std::string host2 = config.get("nsm.global.host");
    int port2 = config.getInt("nsm.global.port");
    NsmbridgeCallback* callback = new NsmbridgeCallback(name);
    NSMNodeDaemon(callback, host, port,
                  new NsmbridgeMasterCallback(callback), host2, port2).run();
  }
  return 0;
}

