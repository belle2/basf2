//+
// File : rf_master.cc
// Description : Master node of RFARM
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 31 - Jul - 2013
//-
#include "daq/rfarm/manager/RFMaster.h"
#include "daq/rfarm/manager/RFMasterCallback.h"
#include "daq/rfarm/manager/RFRunControlCallback.h"

#include <daq/slc/nsm/NSMNodeDaemon.h>
#include <daq/slc/nsm/NSMData.h>
#include <daq/slc/system/PThread.h>
#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/base/Debugger.h>

using namespace std;
using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    Belle2::debug("usage : rf_master <config_file>");
  }
  RFConf conf(argv[1]);
  NSMNode* node = new NSMNode(conf.getconf("master", "nodename"));
  NSMData* data = new NSMData(node->getName(), conf.getconf("system", "nsmdata"), 1);

  RFMaster* master = new RFMaster(argv[1]);

  ConfigFile slc_config("slolwcontrol", "hlt");
  const std::string global_host = slc_config.get("NSM_GLOBAL_HOST");
  const int global_port = slc_config.getInt("NSM_GLOBAL_PORT");
  const std::string local_host = slc_config.get("NSM_LOCAL_HOST");
  const int local_port = slc_config.getInt("NSM_LOCAL_PORT");
  RFMasterCallback* callback = new RFMasterCallback(node, data, master);
  RFRunControlCallback* rccallback = new RFRunControlCallback(new NSMNode("HLT"),
                                                              master, callback);
  PThread(new NSMNodeDaemon(rccallback, global_host, global_port));
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback, local_host, local_port, NULL, data);
  daemon->run();

  return 0;
}
