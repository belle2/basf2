//+
// File : ereco_eventsampler.cc
// Description : Sample events from each ERECO processing nodes and
//               put them in a ring buffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 14 - May - 2017
//-
#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "daq/expreco/ERecoEventSampler.h"

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



