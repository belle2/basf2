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

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/PThread.h>

#include <daq/slc/base/ConfigFile.h>

#include <cstdlib>

using namespace std;
using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    LogFile::debug("usage : %s <config_file>", argv[0]);
  }
  RFConf conf(argv[1]);
  NSMNode node(conf.getconf("master", "nodename"));
  NSMData data(node.getName(), conf.getconf("system", "nsmdata"), 1);

  RFMaster* master = new RFMaster(argv[1]);

  ConfigFile slc_config("slolwcontrol", "hlt");
  const std::string global_host = slc_config.get("nsm.global.host");
  const int global_port = slc_config.getInt("nsm.global.port");
  const std::string local_host = conf.getconf("master", "host");
  const int local_port = atoi(getenv("NSM2_PORT"));
  RFMasterCallback* callback = new RFMasterCallback(node, data, master);
  RFRunControlCallback* rccallback =
    new RFRunControlCallback(NSMNode("HLT"), master, callback);
  if (global_port > 0) {
    PThread(new NSMNodeDaemon(rccallback, global_host, global_port));
  }
  NSMNodeDaemon* daemon = new NSMNodeDaemon(callback, local_host, local_port);
  daemon->run();

  return 0;
}
