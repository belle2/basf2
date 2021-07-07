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

#include <daq/dqm/HistoServer.h>

using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{
  if (argc < 3) {
    printf("Usage : hserver port mapfile\n");
    exit(-1);
  }
  int port = atoi(argv[1]);
  string file = string(argv[2]);

  HistoServer hserv(port, file);

  hserv.init();

  hserv.server();

}


