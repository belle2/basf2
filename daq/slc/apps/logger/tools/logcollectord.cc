#include "daq/slc/apps/logger/LogCollectorCallback.h"

#include "daq/slc/apps/logger/LogDBManager.h"
#include "daq/slc/apps/logger/LogUICommunicator.h"

#include "daq/slc/apps/SocketAcceptor.h"

#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/nsm/NSMNodeDaemon.h>
#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/ConfigFile.h>

#include <daq/slc/system/PThread.h>

using namespace Belle2;

typedef SocketAcceptor<LogUICommunicator, LogDBManager> LogUIAcceptor;

int main(int, char**)
{
  ConfigFile config("slowcontrol");
  std::string nodename = config.get("LOG_NSM_NAME");
  DBInterface* db = new PostgreSQLInterface(config.get("DATABASE_HOST"),
                                            config.get("DATABASE_NAME"),
                                            config.get("DATABASE_USER"),
                                            config.get("DATABASE_PASS"),
                                            config.getInt("DATABASE_PORT"));
  LogDBManager* dbman = new LogDBManager(db, nodename);
  PThread(new LogUIAcceptor(config.get("LOG_GUI_HOST"),
                            config.getInt("LOG_GUI_PORT"), dbman));
  NSMNode* node = new NSMNode(nodename);
  LogCollectorCallback* callback =
    new LogCollectorCallback(node);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback);
  daemon->run();

  return 0;
}
