//+
// File : ereco_event_server.cc
// Description : Send an sampled event to outside over socket
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 17 - Dec - 2013
//-
#include <string>
#include <vector>

//#include <stdio.h>
//#include <stdlib.h>
//#include <unistd.h>

#include "daq/expreco/EventServer.h"

using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{
  // Retrieve RingBuffer name[1], port number[2], Shmname[3], and id[4]
  if (argc < 3) {
    printf("syntax : ereco_eventserver rbufname port\n");
    exit(-1);
  }

  string a1(argv[1]);
  int a2 = atoi(argv[2]);

  EventServer rs(a1, a2);

  rs.server();
}



