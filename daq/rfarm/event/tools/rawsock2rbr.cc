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

#include "daq/rfarm/event/RawRevSock2Rb.h"

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

  RawRevSock2Rb sr(a1, a2, a3, a4, a5);

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
    if (nevt % 5000 == 0) {
      printf("rawsock2rbr : evt = %d\n", nevt);
    }
  }
  exit(0);
}



