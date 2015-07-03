#include "daq/slc/apps/rfarm/RFCallback.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/Daemon.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

typedef void* (*getfee_t)(void);

int main(int argc, char** argv)
{
  if (Daemon::start(argv[1], argc, argv, 3, "<nodename> <hostname> <port>")) {
    //ConfigFile config("slowcontrol");
    std::string host = argv[2];//config.get("nsm.host");
    RFCallback* callback = new RFCallback();
    if (host.size() == 0) {
      LogFile::error("nsm.host is empty");
      return 1;
    }
    int port = atoi(argv[3]);//config.getInt("nsm.port");
    if (port < 0) {
      LogFile::error("nsm.port is not a positive integer");
      return 1;
    }
    std::string name = argv[1];//config.get("nsm.nodename");
    if (name.size() == 0) {
      LogFile::error("nsm.nodename is empty");
      return 1;
    }
    callback->setNode(NSMNode(name));
    NSMNodeDaemon(callback, host, port).run();
  }
  return 0;
}
