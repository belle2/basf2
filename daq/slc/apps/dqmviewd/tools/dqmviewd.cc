#include "daq/slc/apps/dqmviewd/DQMViewCallback.h"
#include "daq/slc/apps/dqmviewd/HistSender.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/PThread.h>
#include <daq/slc/system/Daemon.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <cstring>
#include <cstdlib>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>

using namespace Belle2;

typedef void* MonitorFunc_t(const char*, const char*);

int main(int argc, char** argv)
{
  ConfigFile config("slowcontrol", "dqm");
  const std::string nodename = config.get("nsm.nodename");
  if (!Daemon::start(("dqmviewd." + nodename).c_str(), argc, argv)) {
    return 1;
  }
  DQMViewCallback* callback = new DQMViewCallback(NSMNode(nodename), config);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback,
                                            config.get("nsm.global.host"),
                                            config.getInt("nsm.global.port"));
  daemon->run();
  return 0;
}

