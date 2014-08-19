#include "daq/slc/apps/storagerd/StoragerCallback.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Daemon.h>

#include <daq/slc/base/ConfigFile.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  ConfigFile config("slowcontrol", argv[1]);
  const std::string hostname = config.get("nsm.global.host");
  const int port = config.getInt("nsm.global.port");
  std::string name = config.get("nsm.nodename");
  if (!Daemon::start(("storagerd." + name).c_str(), argc - 1, argv + 1)) {
    return 1;
  }
  NSMNode node(name);
  StoragerCallback* callback = new StoragerCallback(node);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback, hostname, port);
  daemon->run();

  return 0;
}
