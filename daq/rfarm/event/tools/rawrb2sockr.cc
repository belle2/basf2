//+
// File : rawrb2sockr.cc
// Description : Get an event from RingBuffer and send it to socket
//               Reverse Connection
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 25 - Jul - 2013
//-
#include <string>
#include <vector>

//#include <stdio.h>
//#include <stdlib.h>
//#include <unistd.h>

#include "daq/rfarm/event/RawRevRb2Sock.h"

using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{
  // Retrieve RingBuffer name[1], port number[2], Shmname[3], and id[4]
  if (argc < 5) {
    printf("syntax : rb2sock rbufname port\n");
    exit(-1);
  }

  string a1(argv[1]);
  int a2 = atoi(argv[2]);
  string a3(argv[3]);
  int a4 = atoi(argv[4]);

  RawRevRb2Sock rs(a1, a2, a3, a4);

  int nevt = 0;
  for (;;) {
    int stat = rs.SendEvent();
    //    printf ( "rb2sock : sending %d\n", stat );
    if (stat <= 0) {
      printf("rb2sock : error in sending event. The event is lost. Reconnecting....\n");
      rs.Reconnect();
      printf("rb2sock : reconnected.\n");
    }
    nevt++;
    if (nevt % 5000 == 0) {
      printf("rawrb2sockr : evt = %d\n", nevt);
    }
  }
  exit(0);
}



