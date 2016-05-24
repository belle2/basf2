#include "daq/slc/apps/filesenderd/FilesenderCallback.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/Daemon.h>
#include <daq/slc/psql/PostgreSQLInterface.h>

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
    PostgreSQLInterface db(config.get("database.host"),
                           config.get("database.dbname"),
                           config.get("database.user"),
                           config.get("database.password"),
                           config.getInt("database.port"));
    NSMNodeDaemon(new FilesenderCallback(name, config.getInt("expno"), config.getInt("runno"),
                                         config.get("path.ipnp"), config.get("path.pxd"),
                                         config.get("dirs"), config.get("excludes"), db), host, port).run();
  }
  return 0;
}
