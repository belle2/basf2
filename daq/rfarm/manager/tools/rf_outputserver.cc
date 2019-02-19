//+
// File : RFEventServer.cc
// Description : Receive data from event builder 1 and distribute them to
//               Processing nodes
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 24 - June - 2013
//-
#include "daq/rfarm/manager/RFOutputServer.h"
#include "daq/rfarm/manager/RFNSM.h"

using namespace std;
using namespace Belle2;

static RFOutputServer* ots = NULL;

extern "C" void sighandler(int sig)
{
  printf("SIGTERM handler here\n");
  ots->cleanup();
}

int main(int argc, char** argv)
{
  RFConf conf(argv[1]);

  // Creation of event server instance. evs contains the instance
  //  RFOutputServer& ots = RFOutputServer::Create ( argv[1] );

  //  RFOutputServer* ots = new RFOutputServer(argv[1]);
  ots = new RFOutputServer(argv[1]);

  signal(SIGINT, sighandler);
  signal(SIGTERM, sighandler);

  RFNSM nsm(conf.getconf("collector", "nodename"), ots);
  nsm.AllocMem(conf.getconf("system", "nsmdata"));
  ots->SetNodeInfo(nsm.GetNodeInfo());

  ots->server();

}
