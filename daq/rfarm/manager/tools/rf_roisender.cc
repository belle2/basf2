/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/rfarm/manager/RFRoiSender.h"
#include "daq/rfarm/manager/RFNSM.h"

#include <csignal>

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
