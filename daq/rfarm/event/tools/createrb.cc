//+
// File : file2rb.cc
// Description : Get an event from a SeqRoot file and place it in Rbuf
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 28 - Apr - 2012
//-
#include <string>

#include <stdio.h>
#include <stdlib.h>

#include "framework/pcore/RingBuffer.h"

#define RBUFSIZE 100000000
#define MAXEVTSIZE  400000000

using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{
  if (argc < 2) {
    printf("createrb : rbufname \n");
    exit(-1);
  }
  RingBuffer* rbuf = new RingBuffer(argv[1], RBUFSIZE);
}



