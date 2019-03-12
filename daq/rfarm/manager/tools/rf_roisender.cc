//+
// File : rf_roisender.cc
// Description : NSM client to manage ROI sender
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 21 - Nov - 2013
//-
#include "daq/rfarm/manager/RFRoiSender.h"
#include "daq/rfarm/manager/RFNSM.h"

using namespace std;
using namespace Belle2;

static RFRoiSender* roi = NULL;

extern "C" void sighandler(int sig)
{
  printf("SIGTERM handler here\n");
  roi->cleanup();
}

int main(int argc, char** argv)
{
  RFConf conf(argv[1]);

  //  RFRoiSender* roi = new RFRoiSender(argv[1]);
  roi = new RFRoiSender(argv[1]);

  signal(SIGINT, sighandler);
  signal(SIGTERM, sighandler);

  RFNSM nsm(conf.getconf("roisender", "nodename"), roi);
  nsm.AllocMem(conf.getconf("system", "nsmdata"));
  roi->SetNodeInfo(nsm.GetNodeInfo());

  roi->server();

}
