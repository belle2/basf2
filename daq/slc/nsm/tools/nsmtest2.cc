#include <daq/slc/runcontrol/RCCommand.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/database/ConfigObjectTable.h>
#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/StringUtil.h>

#include <iostream>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 3) {
    LogFile::debug("Usage : ./logsender <name> <nodename> <tablename> [configname]\n");
    return 1;
  }
  const std::string name = argv[1];
  const std::string nodename = argv[2];
  const std::string tablename = argv[3];
  const std::string configname = (argc > 4) ? argv[4] : "default";

  ConfigFile dbconfig("slowcontrol");
  DBInterface* db = new PostgreSQLInterface(dbconfig.get("database.host"),
                                            dbconfig.get("database.dbname"),
                                            dbconfig.get("database.user"),
                                            dbconfig.get("database.password"),
                                            dbconfig.getInt("database.port"));
  db->connect();
  ConfigObjectTable table(db);
  ConfigObject obj = table.get(configname, nodename);
  db->close();

  NSMCommunicator* comm = new NSMCommunicator();
  comm->init(NSMNode(nodename));
  NSMNode node(name);
  NSMMessage msg;
  comm->sendRequest(NSMMessage(node, RCCommand::BOOT, obj));
  return 0;
}
