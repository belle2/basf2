#include "daq/slc/apps/runcontrold/RunControlMasterCallback.h"

#include <daq/slc/nsm/NSM2NSMBridge.h>
#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/system/Daemon.h>

#include <daq/slc/base/ConfigFile.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  ConfigFile config("slowcontrol", argv[1]);
  std::string name = config.get("nsm.nodename");
  if (!Daemon::start(("runcontrold." + name).c_str(), argc - 1, argv + 1)) {
    return 1;
  }
  DBInterface* db = new PostgreSQLInterface(config.get("database.host"),
                                            config.get("database.dbname"),
                                            config.get("database.user"),
                                            config.get("database.password"),
                                            config.getInt("database.port"));
  NSMNode node(name);
  RunControlCallback* callback = new RunControlCallback(node);
  RunControlMasterCallback* master_callback =
    new RunControlMasterCallback(node, callback);
  callback->setDB(db);
  NSM2NSMBridge* daemon = new NSM2NSMBridge(callback,
                                            config.get("nsm.local.host"),
                                            config.getInt("nsm.local.port"),
                                            master_callback,
                                            config.get("nsm.global.host"),
                                            config.getInt("nsm.global.port"));
  daemon->run();
  return 0;
}
