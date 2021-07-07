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
#include <unistd.h>

#include "framework/pcore/SeqFile.h"
#include "framework/pcore/RingBuffer.h"
#include "framework/pcore/EvtMessage.h"

#define RBUFSIZE 100000000
#define MAXEVTSIZE 400000000

using namespace Belle2;
using namespace std;

int main(int argc, char** argv)
{
  if (argc < 3) {
    printf("file2rb : rbufname filename neof\n");
    exit(-1);
  }

  SeqFile* file = new SeqFile(argv[2], "w");
  if (file->status() <= 0) {
    perror("file open");
    exit(-1);
  }
  //  RingBuffer* rbuf = new RingBuffer(argv[1], RBUFSIZE);
  RingBuffer* rbuf = new RingBuffer(argv[1]);
  char* evbuf = new char[MAXEVTSIZE];

  int eof = 0;
  for (;;) {
    // Get a record from ringbuf
    int size;
    while ((size = rbuf->remq((int*)evbuf)) == 0) {
      //    printf ( "Rx : evtbuf is not available yet....\n" );
      //      usleep(100);
      usleep(20);
    }
    EvtMessage* msg = new EvtMessage(evbuf);
    if (msg->type() == MSG_TERMINATE) {
      printf("EoF found. Exitting.....\n");
      eof = 1;
    }

    // Put the record in a file
    int wstat = file->write(evbuf);
    if (wstat <= 0) {
      perror("write");
      exit(-99);
    }
    delete msg;
  }
}




