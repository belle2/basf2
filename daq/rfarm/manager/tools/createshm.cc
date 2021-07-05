/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "daq/rfarm/manager/RFSharedMem.h"

using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{
  if (argc < 2) {
    printf("createrb : rbufname \n");
    exit(-1);
  }
  RFSharedMem* shm = new RFSharedMem(argv[1]);
}



