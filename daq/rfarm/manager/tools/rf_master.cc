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

#include <daq/slc/nsm/NSM2NSMBridge.h>
#include <daq/slc/nsm/NSMData.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/ConfigFile.h>

#include <cstdlib>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) {
    LogFile::debug("usage : %s <config_file>", argv[0]);
    return 1;
  }
  RFConf conf(argv[1]);
  NSMNode node(conf.getconf("master", "nodename"));
  NSMData data(node.getName(), conf.getconf("system", "nsmdata"), 1);
  RFMaster* master = new RFMaster(argv[1]);
  ConfigFile file("slowcontrol", "hlt");
  RFMasterCallback* callback = new RFMasterCallback(node, data, master, argv[1]);
  RFRunControlCallback* rccallback =
    new RFRunControlCallback(file.get("nsm.nodename"), master, callback);
  LogFile::debug("%s:%d", file.get("nsm.global.host").c_str(),
                 file.getInt("nsm.global.port"));
  NSM2NSMBridge* daemon = new NSM2NSMBridge(callback,
                                            conf.getconf("master", "host"),
                                            atoi(getenv("NSM2_PORT")),
                                            rccallback,
                                            file.get("nsm.global.host"),
                                            file.getInt("nsm.global.port"));
  daemon->run();

  return 0;
}
