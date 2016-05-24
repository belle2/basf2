#include "daq/slc/apps/logcollectord/LogcollectorCallback.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/psql/PostgreSQLInterface.h>

#include <daq/slc/system/Daemon.h>

#include <daq/slc/base/ConfigFile.h>

#include <iostream>
#include <unistd.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-d") == 0) {
      daemon(0, 0);
    }
  }
  ConfigFile config("slowcontrol");
  std::string host = config.get("nsm.host");
  int port = config.getInt("nsm.port");
  std::string name = config.get("log.collector");
  int timeout = config.getInt("timeout");
  if (timeout <= 0) timeout = 5;
  PostgreSQLInterface db(config.get("database.host"),
                         config.get("database.dbname"),
                         config.get("database.user"),
                         config.get("database.password"),
                         config.getInt("database.port"));
  LogcollectorCallback* callback = new LogcollectorCallback(name, db);
  callback->setDBTable(config.get("log.dbtable"));
  NSMNodeDaemon(callback, host, port).run();
  return 0;
}
