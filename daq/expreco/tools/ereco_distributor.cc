/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/expreco/ERecoDistributor.h"
#include "daq/rfarm/manager/RFNSM.h"

#include <unistd.h>

#include <cstring>

using namespace std;
using namespace Belle2;

int main(int argc, char** argv)
{
  RFConf conf(argv[1]);

  // Creation of event server instance. evs contains the instance
  //  RFEventServer& evs = RFEventServer::Create(argv[1]);

  ERecoDistributor* evs = new ERecoDistributor(argv[1]);


  RFNSM nsm(conf.getconf("distributor", "nodename"), evs);
  nsm.AllocMem(conf.getconf("system", "nsmdata"));
  evs->SetNodeInfo(nsm.GetNodeInfo());

  evs->server();

}
