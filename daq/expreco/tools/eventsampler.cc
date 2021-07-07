/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cstring>
#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>

#include "daq/expreco/EventSampler.h"
#include "daq/rfarm/manager/RFConf.h"

using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{
  // Retrieve RingBuffer name[1], port number[2], Shmname[3], and id[4]
  if (argc < 2) {
    printf("syntax : eventsampler configfile\n");
    exit(-1);
  }

  RFConf* conf = new RFConf(argv[1]);

  int nnodes = 0;
  int maxnodes = conf->getconfi("processor", "nnodes");
  int idbase = conf->getconfi("processor", "idbase");
  char* hostbase = conf->getconf("processor", "ctlhostbase");
  char* badlist = conf->getconf("processor", "badlist");

  vector<string> nodelist;
  char hostname[512], idname[3], shmid[3];
  for (int i = 0; i < maxnodes; i++) {
    sprintf(idname, "%2.2d", idbase + i);
    sprintf(shmid, "%2.2d", i);
    if (badlist == NULL  ||
        strstr(badlist, idname) == 0) {
      sprintf(hostname, "%s%2.2d", hostbase, idbase + i);
      nodelist.push_back(string(hostname));
      nnodes++;
    }
  }

  int port = conf->getconfi("processor", "eventserver", "port");

  char* rbuf = conf->getconf("eventsampler", "ringbufout");
  int interval = conf->getconfi("eventsampler", "interval");

  EventSampler es(nodelist, port, rbuf, interval);

  es.server();
}



