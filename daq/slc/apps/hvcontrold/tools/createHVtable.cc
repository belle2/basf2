#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/StringUtil.h>

#include <daq/slc/apps/hvcontrold/HVControlMaster.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    Belle2::debug("usage : createHVtable <filename> [<nodename>]");
  }
  ConfigFile config(argv[1]);
  HVControlMaster master;
  if (argc > 2) {
    std::string nodename = argv[2];
    master.addNode(new NSMNode(nodename), config.get(nodename + ".include"));
    int id = config.getInt(nodename + ".config");
    master.getNodeInfo(nodename).setConfigId(id);
  } else {
    std::vector<std::string> nodename_v = Belle2::split(config.get("hv.nodes"), ',');
    for (size_t i = 0; i < nodename_v.size(); i++) {
      std::string nodename = nodename_v[i];
      master.addNode(new NSMNode(nodename), config.get(nodename + ".include"));
      int id = config.getInt(nodename + ".config");
      master.getNodeInfo(nodename).setConfigId(id);
    }
  }
  config.read("slowcontrol");
  master.setDB(new PostgreSQLInterface(config.get("database.host"),
                                       config.get("database.dbname"),
                                       config.get("database.user"),
                                       config.get("database.password"),
                                       config.getInt("database.port")));
  master.createTables();
  return 0;
}
