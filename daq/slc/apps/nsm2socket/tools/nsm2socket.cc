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
  if (argc < 1) {
    LogFile::debug("usage: nsm2socket <configname>");
  }
  ConfigFile config("slowcontrol", argv[1]);
  DBInterface* db = new PostgreSQLInterface(config.get("database.host"),
                                            config.get("database.dbname"),
                                            config.get("database.user"),
                                            config.get("database.password"),
                                            config.getInt("database.port"));
  std::string host = config.get("nsm2socket.nsm.host");
  int port = config.getInt("nsm2socket.nsm.port");
  std::string nodename = config.get("nsm2socket.nsm.nodename");
  LogFile::open("nsm2socket." + nodename);
  NSMNode node(nodename);
  NSM2SocketCallback* callback = new NSM2SocketCallback(node, 2, config);
  PThread(new NSMNodeDaemon(callback, host, port));
  host = config.get("nsm2socket.socket.host");
  port = config.getInt("nsm2socket.socket.port");
  NSM2SocketBridge* bridge = new NSM2SocketBridge(host, port, callback, db);
  bridge->run();
  return 1;
}
