//+
// File : RFEventServer.cc
// Description : Receive data from event builder 1 and distribute them to
//               Processing nodes
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 24 - June - 2013
// Daet : 17 - Oct - 2013, Special version for DESY test nodes
//-

//#define DESY

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
#ifndef DESY
  gethostname(&nodename[4], sizeof(nodename));
#else
  // Special treatment for DESY test nodes!!
  char hostnamebuf[256];
  gethostname(hostnamebuf, sizeof(hostnamebuf));
  strcat(&nodename[4], &hostnamebuf[6]);
  int lend = strlen(nodename);
  nodename[lend + 1] = (char)0;
  nodename[lend] = nodename[lend - 1];
  strncpy(&nodename[lend - 1], "0", 1);
  printf("DESY node name = %s\n", nodename);
  // End of DESY special treatment
#endif

  RFNSM nsm(nodename, evproc);
  nsm.AllocMem(conf.getconf("system", "nsmdata"));
  evproc->SetNodeInfo(nsm.GetNodeInfo());

  evproc->server();

}
