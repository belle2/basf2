#include "daq/slc/apps/dqmviewd/DQMViewCallback.h"
#include "daq/slc/apps/dqmviewd/HistSender.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/Daemon.h>

#include <daq/slc/base/ConfigFile.h>

using namespace Belle2;

typedef void* MonitorFunc_t(const char*, const char*);

int main(int argc, char** argv)
{
  if (Daemon::start(argv[1], argc, argv, 1, "<config>")) {
    ConfigFile config("slowcontrol", argv[1]);
    NSMNode node(config.get("nsm.nodename"));
    DQMViewCallback callback(node, config);
    if (node.getName().size() == 0) {
      while (true) {
        sleep(5);
      }
    } else {
      const std::string host = config.get("nsm.host");
      int port = config.getInt("nsm.port");
      NSMNodeDaemon daemon(&callback, host, port);
      daemon.run();
    }
  }
  return 0;
}

