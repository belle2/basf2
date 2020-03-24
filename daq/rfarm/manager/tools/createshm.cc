//+
// File : createshm.cc
// Description : Create Shared Memory for flow logging
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 7 - Apr - 2015
//-

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



