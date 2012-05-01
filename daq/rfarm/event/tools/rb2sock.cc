//+
// File : rb2sock.cc
// Description : Get an event from RingBuffer and send it to socket
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 27 - Apr - 2012
//-
#include <string>
#include <vector>

#include "daq/rfarm/event/Rb2Sock.h"

using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{
  // Retrieve RingBuffer name[1], destination[2], and port number[3]
  if (argc < 3) {
    printf("rb2sock : rbufname, dest, port\n");
    exit(-1);
  }

  Rb2Sock rs(string(argv[1]), string(argv[2]), atoi(argv[3]));

  for (;;) {
    int stat = rs.SendEvent();
    //    printf ( "rb2sock : sending %d\n", stat );
    if (stat <= 0) break;
  }
  exit(0);
}



