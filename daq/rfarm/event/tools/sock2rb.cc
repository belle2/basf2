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



