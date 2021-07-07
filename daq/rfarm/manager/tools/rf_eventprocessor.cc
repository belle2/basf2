/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//#define DESY

#include "daq/rfarm/manager/RFEventProcessor.h"
#include "daq/rfarm/manager/RFNSM.h"

#include <unistd.h>

#include <csignal>
#include <cstring>

using namespace std;
using namespace Belle2;

static RFEventProcessor* evproc = NULL;

extern "C" void sighandler(int sig)
{
  printf("SIGTERM handler here\n");
  evproc->cleanup();
}

int main(int argc, char** argv)
{
  RFConf conf(argv[1]);

  //  RFEventProcessor* evproc = new RFEventProcessor(argv[1]);
  evproc = new RFEventProcessor(argv[1]);

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

  signal(SIGINT, sighandler);
  signal(SIGTERM, sighandler);

  RFNSM nsm(nodename, evproc);
  nsm.AllocMem(conf.getconf("system", "nsmdata"));
  evproc->SetNodeInfo(nsm.GetNodeInfo());

  evproc->server();

}
