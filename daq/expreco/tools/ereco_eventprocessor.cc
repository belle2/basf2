/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//#define DESY

#include "daq/expreco/ERecoEventProcessor.h"
#include "daq/rfarm/manager/RFNSM.h"

#include <unistd.h>

#include <cstring>

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
