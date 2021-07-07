/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include "daq/dataflow/SocketLib.h"

#include <cstdio>
#include <cstdlib>
#include <unistd.h>

using namespace std;
using namespace Belle2;

int main(int argc, char** argv)
{
  SocketSend sock(argv[1], (unsigned short)atoi(argv[2]));

  sleep(10);

  int count = 0;
  for (;;) {
    int sendbuf = 4;
    int is = write(sock.sock(),  &sendbuf, 4);
    printf("data sent (%d), ret=%d\n", count++, is);
    sleep(1);
  }
}
