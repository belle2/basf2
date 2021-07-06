/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <string>

#include <stdio.h>
#include <stdlib.h>

#include "daq/expreco/ERecoEventSampler.h"
#include "daq/rfarm/manager/RFNSM.h"

using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{
  // Retrieve RingBuffer name[1], port number[2], Shmname[3], and id[4]
  if (argc < 2) {
    printf("syntax : ereco_eventsampler configfile\n");
    exit(-1);
  }

  RFConf conf(argv[1]);

  ERecoEventSampler* es = new ERecoEventSampler(argv[1]);

  RFNSM nsm(conf.getconf("eventsampler", "nodename"), es);
  nsm.AllocMem(conf.getconf("system", "nsmdata"));
  es->SetNodeInfo(nsm.GetNodeInfo());


  es->server();
}



