//+
// File : RFEventServer.cc
// Description : Receive data from event builder 1 and distribute them to
//               Processing nodes
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 24 - June - 2013
//-
#include "daq/rfarm/manager/RFDqmServer.h"
#include "daq/rfarm/manager/RFNSM.h"

using namespace std;
using namespace Belle2;

int main(int argc, char** argv)
{
  RFConf conf(argv[1]);

  RFDqmServer* dqm = new RFDqmServer(argv[1]);

  RFNSM nsm(conf.getconf("dqmserver", "nodename"), dqm);
  nsm.AllocMem(conf.getconf("system", "nsmdata"));
  dqm->SetNodeInfo(nsm.GetNodeInfo());

  dqm->server();

}
