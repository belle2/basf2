#include "daq/slc/apps/nsmtemplated/NSMTemplateCallback.h"

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
    NSMNodeDaemon(new NSMTemplateCallback(name), host, port).run();
  }
  return 0;
}
