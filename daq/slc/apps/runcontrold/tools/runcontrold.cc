#include "daq/slc/apps/runcontrold/RunControlMasterCallback.h"

#include <daq/slc/nsm/NSM2NSMBridge.h>
#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>

#include <cstdlib>
#include <unistd.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    LogFile::debug("Usage : %s <config>", argv[0]);
    return 1;
  }
  //daemon(0, 0);
  LogFile::open(argv[0]);
  ConfigFile config("slowcontrol", argv[1]);
  DBInterface* db = new PostgreSQLInterface(config.get("database.host"),
                                            config.get("database.dbname"),
                                            config.get("database.user"),
                                            config.get("database.password"),
                                            config.getInt("database.port"));
  const std::string name = config.get("nsm.nodename");
  NSMNode node(name);
  RunControlCallback* callback = new RunControlCallback(node);
  RunControlMasterCallback* master_callback = new RunControlMasterCallback(node, callback);
  callback->setDB(db);
  /*NSMNodeDaemon* daemon = new NSMNodeDaemon(callback,
                                            config.get("nsm.local.host"),
                                            config.getInt("nsm.local.port"));
  */
  NSM2NSMBridge* daemon = new NSM2NSMBridge(callback,
                                            config.get("nsm.local.host"),
                                            config.getInt("nsm.local.port"),
                                            master_callback,
                                            config.get("nsm.global.host"),
                                            config.getInt("nsm.global.port"));
  daemon->run();
  return 0;
}
