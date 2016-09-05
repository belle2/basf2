//+
// File : rf_master.cc
// Description : Master node of RFARM
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 31 - Jul - 2013
//-
#include "daq/rfarm/manager/RFMaster.h"
#include "daq/rfarm/manager/RFNSM.h"

using namespace std;
using namespace Belle2;

int main(int argc, char** argv)
{
  RFConf conf(argv[1]);

  RFMaster* master = new RFMaster(argv[1]);
  // Creation of event server instance. evs contains the instance
  //  RFOutputServer& ots = RFOutputServer::Create ( argv[1] );

  RFNSM nsm(conf.getconf("master", "nodename"), master);
  nsm.AllocMem(conf.getconf("system", "nsmdata"));
  master->SetNodeInfo(nsm.GetNodeInfo());
  master->Hook_Message_Handlers();

  master->monitor_loop();

}
