/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <string>

#include "daq/rfarm/event/RevRb2Sock.h"

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

  RevRb2Sock rs(a1, a2, a3, a4);

  for (;;) {
    int stat = rs.SendEvent();
    //    printf ( "rb2sock : sending %d\n", stat );
    if (stat <= 0) break;
  }
  exit(0);
}



