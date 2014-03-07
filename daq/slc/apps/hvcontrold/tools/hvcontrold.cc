#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <daq/slc/database/PostgreSQLInterface.h>

#include <daq/slc/nsm/NSMNodeDaemon.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/PThread.h>

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/StringUtil.h>

#include <daq/slc/apps/hvcontrold/HVControlMaster.h>
#include <daq/slc/apps/hvcontrold/HVMasterCallback.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 1) {
    Belle2::debug("usage : hvcontrold");
  }
  ConfigFile config("hv");
  HVControlMaster master;
  std::vector<std::string> nodename_v = Belle2::split(config.get("hv.nodes"), ',');
  for (size_t i = 0; i < nodename_v.size(); i++) {
    std::string nodename = nodename_v[i];
    master.addNode(new NSMNode(nodename), config.get(nodename + ".include"));
    int id = config.getInt(nodename + ".config");
    master.getNodeInfo(nodename).setConfigId(id);
  }
  config.read("slowcontrol");
  master.setDB(new PostgreSQLInterface(config.get("database.host"), config.get("database.dbname"),
                                       config.get("database.user"), config.get("database.password"),
                                       config.getInt("database.port")));
  master.createTables();
  LogFile::open("hvcontrold");

  NSMNode* node = new NSMNode(config.get("hv.nsm.name"));
  HVMasterCallback* callback = new HVMasterCallback(node, &master);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback, config.get("nsm.local.host"),
                                            config.getInt("nsm.local.port"));
  daemon->run();

  return 0;
}
