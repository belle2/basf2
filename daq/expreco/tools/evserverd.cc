#include "daq/expreco/EventServerCallback.h"

#include <daq/slc/psql/PostgreSQLInterface.h>

#include <daq/slc/runcontrol/RCNodeDaemon.h>

#include <daq/slc/base/ConfigFile.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (Daemon::start(argv[1], argc, argv, 1, "<config>")) {
    ConfigFile config("slowcontrol", argv[1]);
    RCNodeDaemon(config, new EventServerCallback(), NULL,
                 new PostgreSQLInterface(config.get("database.host"),
                                         config.get("database.dbname"),
                                         config.get("database.user"),
                                         config.get("database.password"),
                                         config.getInt("database.port"))).run();
  }
  return 0;
}
