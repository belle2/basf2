//+
// File : hserver.cc
// Description : A server to collect histograms over network
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 23 - Apr - 2013
//-
#include <string>
#include <stdio.h>
#include <stdlib.h>

#include <daq/dqm/HistoServer.h>

using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{
  if (argc < 3) {
    printf("Usage : hserver port mapfile\n");
    exit(-1);
  }
  int port = atoi(argv[1]);
  string file = string(argv[2]);

  HistoServer hserv(port, file);

  hserv.init();

  hserv.server();

}


