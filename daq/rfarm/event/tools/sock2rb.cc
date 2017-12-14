//+
// File : sock2rb.cc
// Description : Get an event from RingBuffer and send it to socket
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 27 - Apr - 2012
//-
#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "daq/rfarm/event/Sock2Rb.h"

using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{
  // Retrieve RingBuffer name[1], and port number[2]
  if (argc < 2) {
    printf("sock2rb : rbufname, port\n");
    exit(-1);
  }

  Sock2Rb sr(string(argv[1]), atoi(argv[2]));

  for (;;) {
    int stat = sr.ReceiveEvent();
    //    printf ( "sock2rb received : %d\n", stat );
    if (stat <= 0) break;
  }
  exit(0);
}



