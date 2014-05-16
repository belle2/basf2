#include "daq/slc/apps/nsm2socket/NSM2SocketBridge.h"
#include "daq/slc/apps/nsm2socket/NSM2SocketCallback.h"

#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/PThread.h>
#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 3) {
    LogFile::debug("usage: %s <nodename> <port> [<hostname2nsm> <port2nsm>]", argv[0]);
    return 1;
  }
  ConfigFile config("slowcontrol");
  DBInterface* db = new PostgreSQLInterface(config.get("database.host"),
                                            config.get("database.dbname"),
                                            config.get("database.user"),
                                            config.get("database.password"),
                                            config.getInt("database.port"));
  const std::string nodename = argv[1];
  const std::string host = "0.0.0.0";
  const int port = atoi(argv[2]);
  LogFile::open("nsm2socket." + nodename);
  NSMNode node(nodename);
  NSM2SocketCallback* callback = new NSM2SocketCallback(node, 2);
  const std::string host2nsm = (argc > 4) ? argv[3] : "";
  const int port2nsm = (argc > 4) ? atoi(argv[4]) : -1;
  PThread(new NSMNodeDaemon(callback, host2nsm, port2nsm));
  NSM2SocketBridge* bridge = new NSM2SocketBridge(host, port, callback, db);
  bridge->run();
  return 1;
}
