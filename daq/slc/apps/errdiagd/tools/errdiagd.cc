#include "daq/slc/apps/errdiagd/ErrdiagCallback.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/Daemon.h>

#include <daq/slc/base/ConfigFile.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (Daemon::start(argv[1], argc, argv, 2, "<config> <xml path>")) {
    ConfigFile config("slowcontrol", argv[1]);
    std::string host = config.get("nsm.host");
    int port = config.getInt("nsm.port");
    std::string name = config.get("nsm.nodename");
    int timeout = config.getInt("timeout");
    if (timeout <= 0) timeout = 5;
    //    std::string xmlpath = "/home/usr/yamadas/slc/database/tools/temp2.xml";
    std::string xmlpath = argv[2];
    //    printf("errdiag : %s\n", xmlpath.c_str() );
    NSMNodeDaemon(new ErrdiagCallback(name, xmlpath), host, port).run();
  }
  return 0;
}
