#include "daq/slc/runcontrol/RunControlMasterCallback.h"

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
    LogFile::debug("Usage : %s <name>", argv[0]);
    return 1;
  }
  //daemon(0, 0);
  LogFile::open(argv[0]);
  ConfigFile config("slowcontrol");
  DBInterface* db = new PostgreSQLInterface(config.get("database.host"),
                                            config.get("database.dbname"),
                                            config.get("database.user"),
                                            config.get("database.password"),
                                            config.getInt("database.port"));
  const char* name = argv[1];
  NSMNode node(name);
  RunControlMasterCallback* callback = new RunControlMasterCallback(node);
  callback->setDB(db);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback);
  daemon->run();
  return 0;
}
