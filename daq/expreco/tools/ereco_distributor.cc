//+
// File : ERecoDistributor.cc
// Description : Receive data from storage and distribute them to
//               Processing nodes
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 24 - June - 2013
//-
#include "daq/expreco/ERecoDistributor.h"
#include "daq/rfarm/manager/RFNSM.h"

using namespace std;
using namespace Belle2;

int main(int argc, char** argv)
{
  RFConf conf(argv[1]);

  // Creation of event server instance. evs contains the instance
  //  RFEventServer& evs = RFEventServer::Create(argv[1]);

  ERecoDistributor* evs = new ERecoDistributor(argv[1]);


  RFNSM nsm(conf.getconf("distributor", "nodename"), evs);
  nsm.AllocMem(conf.getconf("system", "nsmdata"));
  evs->SetNodeInfo(nsm.GetNodeInfo());

  evs->server();

}
