/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/rfarm/manager/RFEventServer.h"
#include "daq/rfarm/manager/RFNSM.h"

#include <csignal>

using namespace std;
using namespace Belle2;

static RFEventServer* evs = NULL;

extern "C" void sighandler(int sig)
{
  printf("SIGTERM handler here\n");
  evs->cleanup();
}

int main(int argc, char** argv)
{
  RFConf conf(argv[1]);

  // Creation of event server instance. evs contains the instance
  //  RFEventServer& evs = RFEventServer::Create(argv[1]);

  //  RFEventServer* evs = new RFEventServer(argv[1]);
  evs = new RFEventServer(argv[1]);

  signal(SIGINT, sighandler);
  signal(SIGTERM, sighandler);
  printf("Signal handler installed\n");

  RFNSM nsm(conf.getconf("distributor", "nodename"), evs);
  nsm.AllocMem(conf.getconf("system", "nsmdata"));
  evs->SetNodeInfo(nsm.GetNodeInfo());

  evs->server();

}
