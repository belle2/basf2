//+
// File : hserver.cc
// Description : A server to collect histograms over network
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 23 - Apr - 2013
//-
#include <stdio.h>
#include <stdlib.h>

#include <daq/dqm/HistoServer.h>

#define HISTO_PORT 9998

int main(int argc, char** argv)
{
  HistoServer hserv(HISTO_PORT);

  hserv.init();

  hserv.server();

}


