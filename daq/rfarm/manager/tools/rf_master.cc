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

using namespace std;
using namespace Belle2;

int main(int argc, char** argv)
{
  RFConf conf(argv[1]);

  RFMaster* master = new RFMaster(argv[1]);

  ConfigFile slc_config("slc_config");
  PThread(new NSMNodeDaemon(new RFRunControlCallback(new NSMNode("HLT"), master),
                            slc_config.get("RC_NSM_GLOBAL_HOST"),
                            slc_config.getInt("RC_NSM_GLOBAL_PORT")));
  NSMNode* node = new NSMNode(conf.getconf("master", "nodename"));
  NSMData* data = new NSMData(node->getName(), conf.getconf("system", "nsmdata"), 1);
  NSMNodeDaemon* daemon = new NSMNodeDaemon(new RFMasterCallback(node, data, master));
  daemon->setWData(data);
  daemon->run();

  return 0;
}
