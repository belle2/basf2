/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include <daq/dqm/HistoRelay.h>

using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{
  if (argc < 4) {
    printf("Usage : hrelay mapfile dest port interval\n");
    exit(-1);
  }
  string mapfile = string(argv[1]);
  string dest = string(argv[2]);
  int port = atoi(argv[3]);
  int interval = atoi(argv[4]);

  HistoRelay hrelay(mapfile, dest, port);

  for (;;) {
    sleep(interval);
    //    printf("hrelay : collecting histograms\n");
    hrelay.collect();
  }

}


