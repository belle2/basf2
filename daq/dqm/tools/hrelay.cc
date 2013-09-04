//+
// File : hrelay.cc
// Description : A server to collect histograms over network
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 23 - Apr - 2013
//-
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
    printf("hrelay : collecting histograms\n");
    hrelay.collect();
  }

}


