//+
// File : rb2rb.cc
// Description : move data between two RingBuffers
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 29 - Apr - 2012
//-

#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "framework/pcore/RingBuffer.h"
#include "framework/pcore/EvtMessage.h"

#define RBUFSIZE 100000000
#define MAXEVTSIZE  400000000

using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{
  if (argc < 3) {
    printf("file2rb : inrbufname outrbufname\n");
    exit(-1);
  }

  //  RingBuffer* rbufin = new RingBuffer(argv[1], RBUFSIZE);
  RingBuffer* rbufin = new RingBuffer(argv[1]);
  //  RingBuffer* rbufout = new RingBuffer(argv[2], RBUFSIZE);
  RingBuffer* rbufout = new RingBuffer(argv[2]);
  char* evbuf = new char[MAXEVTSIZE];

  int eof = 0;
  while (eof == 0) {
    // Get a record from ringbuf
    int bsize;
    while ((bsize = rbufin->remq((int*)evbuf)) == 0) {
      //    printf ( "Rx : evtbuf is not available yet....\n" );
      //      usleep(100);
      usleep(20);
    }
    EvtMessage* msg = new EvtMessage(evbuf);
    if (msg->type() == MSG_TERMINATE) {
      printf("EoF found. Exitting.....\n");
      eof = 1;
    }

    // Put the message in ring buffer
    int irb = 0;
    for (;;) {
      irb = rbufout->insq((int*)evbuf, bsize);
      if (irb >= 0) break;
      //      usleep(100);
      usleep(20);
    }
  }
}




