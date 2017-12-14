//+
// File : rb2mrb.cc
// Description : deliver events in a RingBuffer to multiple RingBuffers
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
    printf("file2rb : inrbufname outrbufname1 outbufname2 .....\n");
    exit(-1);
  }

  //  RingBuffer* rbufin = new RingBuffer(argv[1], RBUFSIZE);
  RingBuffer* rbufin = new RingBuffer(argv[1]);
  //  RingBuffer* rbufout = new RingBuffer(argv[2], RBUFSIZE);

  int nout = argc - 2;

  vector<RingBuffer*> rbufout;
  for (int i = 0; i < nout; i++) {
    rbufout.push_back(new RingBuffer(argv[i + 2]));
  }

  char* evbuf = new char[MAXEVTSIZE];

  int outptr = 0;
  int bsize;
  for (;;) {
    // Get a record from ringbuf
    while ((bsize = rbufin->remq((int*)evbuf)) == 0) {
      //    printf ( "Rx : evtbuf is not available yet....\n" );
      //      usleep(100);
      usleep(20);
    }
    EvtMessage* msg = new EvtMessage(evbuf);
    if (msg->type() == MSG_TERMINATE) {
      printf("EoF found. Exitting.....\n");
      break;
    }

    // Put the message in ring buffer
    int irb = 0;
    for (;;) {
      irb = rbufout[outptr]->insq((int*)evbuf, bsize);
      if (irb >= 0) break;
      //      usleep(100);
      usleep(20);
    }
    outptr++;
    if (outptr >= nout) outptr = 0;
    delete msg;
  }

  // EOF
  for (int i = 0; i < nout; i++) {
    int irb = 0;
    for (;;) {
      irb = rbufout[i]->insq((int*)evbuf, bsize);
      if (irb >= 0) break;
      //      usleep(100);
      usleep(20);
    }
  }
  exit(0);
}




