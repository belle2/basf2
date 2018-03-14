//+
// File : ERecoEventServer.cc
// Description : Receive data from event builder 1 and distribute them to
//               Processing nodes
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 24 - June - 2013
// Daet : 17 - Oct - 2013, Special version for DESY test nodes
//-

//#define DESY

#include "daq/expreco/ERecoEventProcessor.h"
#include "daq/rfarm/manager/RFNSM.h"

using namespace std;
using namespace Belle2;

int main(int argc, char** argv)
{
  RFConf conf(argv[1]);

  ERecoEventProcessor* evproc = new ERecoEventProcessor(argv[1]);

  char nodename[256];
  strcpy(nodename, "evp_");
  gethostname(&nodename[4], sizeof(nodename));

  RFNSM nsm(nodename, evproc);
  nsm.AllocMem(conf.getconf("system", "nsmdata"));
  evproc->SetNodeInfo(nsm.GetNodeInfo());

  evproc->server();

}
