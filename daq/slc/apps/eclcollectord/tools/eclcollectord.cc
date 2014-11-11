#include "daq/slc/apps/eclcollectord/ECLCollectorCallback.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Daemon.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  ConfigFile config("slowcontrol", argv[1]);
  const std::string hostname = config.get("nsm.local.host");
  const int port = config.getInt("nsm.local.port");
  const std::string name = config.get("nsm.nodename");
  const std::string colhostname = config.get("ecl.collector.host");
  if (!Daemon::start(("eclcollectord." + name).c_str(), argc - 1, argv + 1)) {
    return 1;
  }
  NSMNode node(name);
  ECLCollectorCallback* callback = new ECLCollectorCallback(node, colhostname);

  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback, hostname, port);
  daemon->run();

  return 0;
}
