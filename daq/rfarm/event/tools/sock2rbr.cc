//+
// File : sock2rbr.cc
// Description : Get an event from RingBuffer and send it to socket
//               Reverse connection
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 25 - Jul - 2013
//-
#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "daq/rfarm/event/RevSock2Rb.h"

using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{
  // Retrieve RingBuffer name[1], port number[2], shmname[4], and id[5]
  if (argc < 2) {
    printf("syntax : sock2rb rbufname src port\n");
    exit(-1);
  }

  string a1(argv[1]);
  string a2(argv[2]);
  int a3 = atoi(argv[3]);
  string a4(argv[4]);
  int a5 = atoi(argv[5]);

  RevSock2Rb sr(a1, a2, a3, a4, a5);

  int ncount = 0;
  int nevt = 0;
  for (;;) {
    int stat = sr.ReceiveEvent();
    //    printf ( "sock2rb received : %d\n", stat );
    if (stat < 0)
      break;
    else if (stat == 0) {
      // Reconnection needed
      int nrepeat = 5000;
      for (;;) {
        int rstat = sr.Reconnect(nrepeat);
        if (rstat == -1)
          continue;
        else
          break;
      }
    }
    nevt++;
    /*
    if (nevt % 5000 == 0) {
      printf("sock2rbr : evt = %d\n", nevt);
    }
    */
  }
  exit(0);
}



