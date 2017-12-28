#include "daq/slc/apps/errdiagd/ErrdiagCallback.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/Daemon.h>

#include <daq/slc/base/ConfigFile.h>

using namespace Belle2;

int main(int argc, char** argv)
{

  if (argc != 5) {
    printf("Usage : %s <slc config file> <timeout(s)> <max entries> <offset entries>\n", argv[0]);
    exit(1);
  }
  if (Daemon::start(argv[1], argc, argv, 1, "<config>")) {
    ConfigFile config("slowcontrol", argv[1]);
    std::string host = config.get("nsm.host");
    int port = config.getInt("nsm.port");
    std::string name = config.get("nsm.nodename");
    //     int timeout = config.getInt("timeout");
    int timeout = atoi(argv[2]);
    if (timeout <= 0) timeout = 5;
    const char* path = getenv("BELLE2_LOCAL_DIR");
    std::string xmlpath = path;
    xmlpath = xmlpath + "/daq/slc/data/errdiag/CPRROPC_Diagnosis.xml";

    NSMNodeDaemon(new ErrdiagCallback(name, xmlpath, timeout, atoi(argv[3]), atoi(argv[4])), host, port).run();
  }
  return 0;
}
