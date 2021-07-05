/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/rfarm/manager/RFDqmServer.h"
#include "daq/rfarm/manager/RFNSM.h"

#include <csignal>

using namespace std;
using namespace Belle2;

static RFDqmServer* dqm = NULL;

extern "C" void sighandler(int sig)
{
  printf("SIGTERM handler here\n");
  dqm->cleanup();
}

int main(int argc, char** argv)
{
  RFConf conf(argv[1]);

  //  RFDqmServer* dqm = new RFDqmServer(argv[1]);
  dqm = new RFDqmServer(argv[1]);

  signal(SIGINT, sighandler);
  signal(SIGTERM, sighandler);

  RFNSM nsm(conf.getconf("dqmserver", "nodename"), dqm);
  nsm.AllocMem(conf.getconf("system", "nsmdata"));
  dqm->SetNodeInfo(nsm.GetNodeInfo());

  dqm->server();

}
