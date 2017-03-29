//+
// File : mrb2rb.cc
// Description : copy events from multiple RingBuffers to a single RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 5 - Aug - 2014
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
    printf("file2rb : inrbufname1 inrbufname2 ....  outrbufname\n");
    exit(-1);
  }

  int nin = argc - 2;

  vector<RingBuffer*> rbufin;
  for (int i = 0; i < nin; i++) {
    rbufin.push_back(new RingBuffer(argv[i + 1]));
  }
  RingBuffer* rbufout = new RingBuffer(argv[argc - 1]);
  char* evbuf = new char[MAXEVTSIZE];

  int inptr = 0;
  for (;;) {
    // Get a record from ringbuf
    int bsize;
    while ((bsize = rbufin[inptr]->remq((int*)evbuf)) == 0) {
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
      irb = rbufout->insq((int*)evbuf, bsize);
      if (irb >= 0) break;
      //      usleep(100);
      usleep(20);
    }
    inptr++;
    if (inptr >= nin) inptr = 0;
    delete msg;
  }
}




