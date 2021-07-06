/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <string>

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



