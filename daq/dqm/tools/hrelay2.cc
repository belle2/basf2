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
#include <unistd.h>
#include <TFile.h>

#include <daq/dqm/HistoRelay2.h>

using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{
  if (argc < 4) {
    printf("Usage : hrelay2 file dest port interval\n");
    exit(-1);
  }
  string file = string(argv[1]);
  string dest = string(argv[2]);
  int port = atoi(argv[3]);
  int interval = atoi(argv[4]);

  // Create empty file before starting
  auto filepath = string("/dev/shm/") + file;
  auto dqmhisto = new TFile(filepath.c_str(), "NEW");
  if (dqmhisto == nullptr) {
    printf("Failed to allocate TFile %s", filepath.c_str());
    exit(-1);
  } else {
    delete dqmhisto;
  }

  HistoRelay2 hrelay2(file, dest, port);

  for (;;) {
    //    printf("hrelay2 : collecting histograms\n");
    if (hrelay2.collect()) {
      sleep(1); // just short if file is just updated
    } else {
      sleep(interval);
    }
  }

}
