//+
// File : RFEventServer.cc
// Description : Receive data from event builder 1 and distribute them to
//               Processing nodes
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 24 - June - 2013
//-
#include "daq/rfarm/manager/RFEventProcessor.h"
#include "daq/rfarm/manager/RFNSM.h"

using namespace std;
using namespace Belle2;

int main(int argc, char** argv)
{
  RFConf conf(argv[1]);

  RFEventProcessor* evproc = new RFEventProcessor(argv[1]);

  char nodename[256];
  strcpy(nodename, "evp_");
  gethostname(&nodename[4], sizeof(nodename));
  RFNSM nsm(nodename, evproc);
  nsm.AllocMem(conf.getconf("system", "nsmdata"));
  evproc->SetNodeInfo(nsm.GetNodeInfo());

  evproc->server();

}
