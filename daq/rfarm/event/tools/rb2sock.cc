/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <string>

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

  string a1(argv[1]);
  string a2(argv[2]);
  int a3 = atoi(argv[3]);

  Rb2Sock rs(a1, a2, a3);

  for (;;) {
    int stat = rs.SendEvent();
    //    printf ( "rb2sock : sending %d\n", stat );
    if (stat <= 0) break;
  }
  exit(0);
}



